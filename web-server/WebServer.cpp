#include "WebServer.h"

namespace web_server {
	WebServer::WebServer(int maxClients, int serverPort) {
		this->maxClients = maxClients;
		this->serverPort = serverPort;
		this->logger = &Logger::instance();
	}

	void WebServer::run() {
		// Init Server's Listen Socket
		SOCKET listenSocketId = initListenSocket();

		if (listenSocketId == NULL) {
			return;
		}

		fd_set waitRecv, waitSend;
		int numOfFD;

		while (true) {
			numOfFD = selectSockets(&waitRecv, &waitSend);
			if (SOCKET_ERROR == numOfFD) {
				logger->log(Err, "Server: Error at select(): " + to_string(WSAGetLastError()));
				WSACleanup();
				return;
			}
			handleClients(numOfFD, &waitRecv, &waitSend);
		}

		logger->log(Info, "Server: Server Shutting Down.");
		closesocket(listenSocketId);
		WSACleanup();
	}

	SOCKET WebServer::initListenSocket() {
		WSAData wsaData;

		if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			logger->log(Err, "Server: Error at WSAStartup()");
			return NULL;
		}

		SOCKET socketID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == socketID) {
			logger->log(Err, "Server: Error at socket(): " + to_string(WSAGetLastError()));
			WSACleanup();
			return NULL;
		}

		sockaddr_in serverService;

		serverService.sin_addr.s_addr = INADDR_ANY;
		serverService.sin_family = AF_INET;
		serverService.sin_port = htons(serverPort);

		if (SOCKET_ERROR == bind(socketID, (SOCKADDR*)&serverService, sizeof(serverService))) {
			logger->log(Err, "Server: Error at bind(): " + to_string(WSAGetLastError()));
			closesocket(socketID);
			WSACleanup();
			return NULL;
		}

		if (SOCKET_ERROR == listen(socketID, 5)) {
			logger->log(Err, "Server: Error at listen(): " + to_string(WSAGetLastError()));
			closesocket(socketID);
			WSACleanup();
			return NULL;
		}

		logger->log(Info, "Server is ready and running on address " + string(inet_ntoa(serverService.sin_addr)) + ":" + to_string(serverPort));

		addNewClient(socketID, RecvMod::LISTEN);

		return socketID;
	}

	bool WebServer::addNewClient(SOCKET id, int recvStatus) {
		if (clients.size() < maxClients) {
			Socket* newSocket = new Socket(id);
			clients.push_back(Client(newSocket, (RecvMod)recvStatus));
			return true;
		}
		return false;
	}

	int WebServer::selectSockets(fd_set* waitRecv, fd_set* waitSend) {
		FD_ZERO(waitRecv);
		FD_ZERO(waitSend);

		for (auto iter = clients.begin(); iter != clients.end(); ++iter) {
			if (iter->getRecvMod() == LISTEN || iter->getRecvMod() == RECEIVE)
				FD_SET(iter->getSocket()->getSocketID(), waitRecv);
			if (iter->hasReadyResponses())
				FD_SET(iter->getSocket()->getSocketID(), waitSend);
		}

		return select(0, waitRecv, waitSend, NULL, &SELECT_TIME_OUT_VAL);
	}

	void WebServer::handleClients(int numOfFD, fd_set* waitRecv, fd_set* waitSend) {
		

		for (auto iter = clients.begin(); iter != clients.end(); ++iter) {
			if (numOfFD > 0) {
				handleWaitRecv(iter, waitRecv, numOfFD);
				handleWaitSend(iter, waitSend, numOfFD);
			}
			handleInComingRequests(iter);
			handleTimeOut(iter, now);
		}

		deleteSockets();
	}

	void WebServer::handleInComingRequests(list<Socket>::iterator& socketIterator) {
		if ((*socketIterator).getSocketState().send != HANDLE_REQ) {
			return;
		}

		Request req = RequestParser::ParseRequest((*socketIterator).getInComingResponse());
		Response response = ResponseCreator::CreateResponse(req);
		// socket.setSocketState.send(sendResponse)
	}

	void WebServer::handleWaitRecv(list<Socket>::iterator& socketIterator, fd_set* waitRecv, int& numOfFD) {
		if (FD_ISSET((*socketIterator).getSocketID(), waitRecv)) {
			numOfFD--;
			switch ((*socketIterator).getSocketState().recv) {
			case LISTEN:
				acceptConnection(*socketIterator);
				break;

			case RECEIVE:
				// false if connection needs to shut down
				if (false == receiveMessage(*socketIterator)) {
					socketsToDelete.push_back(socketIterator);
				}
				break;
			}
		}
	}

	void WebServer::handleWaitSend(list<Client>::iterator& client, fd_set* waitSend, int& numOfFD) {
		if (FD_ISSET(client->getSocketID(), waitSend)) {
			numOfFD--;
			bool err = client->sendResponses();
			if (err){
				logger->log(Err, "Server: Error at send(): " + to_string(WSAGetLastError()));
			}
			else {
				logger->log(Info, "Server: Sent message.");
			}
		}
	}

	void WebServer::handleTimeOut(list<Socket>::iterator& socketIterator, time_t now) {
		time_t now;
		time(&now);

		time_t timePassed = now - (*socketIterator).getSocketState().lastRecvTime;

		if (timePassed >= TIME_OUT && (*socketIterator).getSocketState().recv == RECEIVE) {
			logger->log(Info, "Server: Client Timed Out, disconnecting...");
			clientsToDelete.push_back(socketIterator);
		}
	}

	void WebServer::deleteSockets() {
		auto it = clientsToDelete.begin();
		while (it != clientsToDelete.end()) {
			printDisconnectSocket((*it)->getSocket()->getSocketID());
			closesocket((*it)->getSocket()->getSocketID());
			clients.erase((*it));
			it++;
		}

		clientsToDelete.clear();
	}

	void WebServer::acceptConnection(Socket& socket) {
		struct sockaddr_in from;
		int fromLen = sizeof(from);

		SOCKET msgSocket = accept(socket.getSocketID(), (struct sockaddr*)&from, &fromLen);
		if (INVALID_SOCKET == msgSocket) {
			logger->log(Err, "Server: Error at accept(): " + to_string(WSAGetLastError()));
			return;
		}
		logger->log(Info, "Server: Client " + string(inet_ntoa(from.sin_addr)) + ":" + to_string(ntohs(from.sin_port)) + " is connected.");
		
		//
		// Set the socket to be in non-blocking mode.
		//
		unsigned long flag = 1;
		if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0) {
			logger->log(Err, "Server: Error at ioctlsocket(): " + to_string(WSAGetLastError()));
		}

		if (addSocket(msgSocket, RECEIVE) == false) {
			logger->log(Info, "Too many connections, dropped!");
			closesocket(socket.getSocketID());
		}
	}

	bool WebServer::receiveMessage(Socket& socket) {
		char buffer[MAX_MSG_SIZE];
		SOCKET msgSocket = socket.getSocketID();

		int bytesRecv = recv(msgSocket, buffer, sizeof(buffer), 0);

		if (SOCKET_ERROR == bytesRecv) {
			logger->log(Err, "Server: Error at recv(): " + to_string(WSAGetLastError()));
			return false;
		}

		if (bytesRecv == 0) {
			return false;
		}

		else {
			buffer[bytesRecv] = '\0'; //add the null-terminating to make it a string
			logger->log(Info, "Server: Recieved: " + bytesRecv + string(" bytes of \"") + buffer + "\" message.");
			socket.setSocketLastRecv();
			socket.setSocketSendState(HANDLE_REQ);
			pendingRequests.push(RequestParser::ParseRequest(buffer));
			return true;
		}
	}

	void WebServer::sendMessage(SOCKET sokcetPtr) {
		int bytesSent = 0;

		SOCKET msgSocket = (*sokcetPtr).getSocketID();

	
	}

	void WebServer::printDisconnectSocket(const SOCKET& socket) {
		struct sockaddr_in name;
		int nameLen = sizeof(name);

		if (getpeername(socket, (struct sockaddr*)&name, &nameLen) == SOCKET_ERROR) {
			logger->log(Err, "Server: Error at getpeername(): " + to_string(WSAGetLastError()));
		}
		logger->log(Info, "Server: Client "+string(inet_ntoa(name.sin_addr)) + ":" +\
			        to_string(ntohs(name.sin_port)) +" has disconnected.");
	}
}