#include "WebServer.h"

namespace web_server {
	WebServer::WebServer(int maxSockets, int serverPort) {
		this->maxSockets = maxSockets;
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
				cerr << "Server: Error at select(): " << WSAGetLastError() << endl;
				WSACleanup();
				return;
			}
			handleSockets(numOfFD, &waitRecv, &waitSend);
		}

		logger->log(Info, "Server: Server Shutting Down.");
		closesocket(listenSocketId);
		WSACleanup();
	}
	

	SOCKET WebServer::initListenSocket() {
		WSAData wsaData;

		if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			cerr << "Server: Error at WSAStartup()\n";
			return NULL;
		}

		SOCKET socketID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET == socketID) {
			cerr << "Server: Error at socket(): " << WSAGetLastError() << endl;
			WSACleanup();
			return NULL;
		}

		sockaddr_in serverService;

		serverService.sin_addr.s_addr = INADDR_ANY;
		serverService.sin_family = AF_INET;
		serverService.sin_port = htons(serverPort);

		if (SOCKET_ERROR == bind(socketID, (SOCKADDR*)&serverService, sizeof(serverService))) {
			cerr << "Server: Error at bind(): " << WSAGetLastError() << endl;
			closesocket(socketID);
			WSACleanup();
			return NULL;
		}

		if (SOCKET_ERROR == listen(socketID, 5)) {
			cerr << "Server: Error at listen(): " << WSAGetLastError() << endl;
			closesocket(socketID);
			WSACleanup();
			return NULL;
		}

		ss << "Server is ready and running on address " << inet_ntoa(serverService.sin_addr) << ":" << serverPort;
		logger->log(Info, ss.str());
		ss.str("");
		ss.clear();

		addSocket(socketID, LISTEN);

		return socketID;
	}

	bool WebServer::addSocket(SOCKET id, int recvStatus) {
		if (sockets.size() < maxSockets) {
			Socket newSocket(id, recvStatus);
			sockets.push_back(newSocket);
			return true;
		}
		return false;
	}

	int WebServer::selectSockets(fd_set* waitRecv, fd_set* waitSend) {
		FD_ZERO(waitRecv);
		FD_ZERO(waitSend);

		list<Socket>::iterator it;

		for (it = sockets.begin(); it != sockets.end(); ++it) {
			SocketState currSocketState = (*it).getSocketState();
			if (currSocketState.recv == LISTEN || currSocketState.recv == RECEIVE)
				FD_SET((*it).getSocketID(), waitRecv);
			if (currSocketState.send == SEND_RESPONSE)
				FD_SET((*it).getSocketID(), waitSend);
		}

		return select(0, waitRecv, waitSend, NULL, &SELECT_TIME_OUT_VAL);
	}

	void WebServer::handleSockets(int numOfFD, fd_set* waitRecv, fd_set* waitSend) {
		list<Socket>::iterator socketIterator;

		time_t now;
		time(&now);

		for (socketIterator = sockets.begin(); socketIterator != sockets.end(); ++socketIterator) {
			if (numOfFD > 0) {
				handleWaitRecv(socketIterator, waitRecv, numOfFD);
				handleWaitSend(socketIterator, waitSend, numOfFD);
			}

			handleTimeOut(socketIterator, now);
		}

		deleteSockets();
	}

	void WebServer::handleWaitRecv(list<Socket>::iterator& socketIterator, fd_set * waitRecv, int& numOfFD) {
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

	void WebServer::handleWaitSend(list<Socket>::iterator& socketIterator, fd_set * waitSend, int& numOfFD) {
		if (FD_ISSET((*socketIterator).getSocketID(), waitSend)) {
			numOfFD--;
			sendMessage(&(*socketIterator));
		}
	}
	void WebServer::handleTimeOut(list<Socket>::iterator& socketIterator, time_t now) {
		time_t timePassed = now - (*socketIterator).getSocketState().lastRecvTime;

		if (timePassed >= TIME_OUT && (*socketIterator).getSocketState().recv == RECEIVE) {
			logger->log(Info, "Server: Client Timed Out, disconnecting...");
			socketsToDelete.push_back(socketIterator);
		}
	}

	void WebServer::deleteSockets() {
		auto it = socketsToDelete.begin();
		while (it != socketsToDelete.end()) {
			printDisconnectSocket((*(*it)).getSocketID());
			closesocket((*(*it)).getSocketID());
			sockets.erase((*it));
			it++;
		}

		socketsToDelete.clear();
	}

	void WebServer::acceptConnection(Socket& socket) {
		struct sockaddr_in from;
		int fromLen = sizeof(from);

		SOCKET msgSocket = accept(socket.getSocketID(), (struct sockaddr*)&from, &fromLen);
		if (INVALID_SOCKET == msgSocket) {
			ss << "Server: Error at accept(): " << WSAGetLastError() << endl;
			logger->log(Err, ss.str());
			ss.str("");
			ss.clear();
			return;
		}
		ss << "Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected.";
		logger->log(Info, ss.str());
		ss.str("");
		ss.clear();

		//
		// Set the socket to be in non-blocking mode.
		//
		unsigned long flag = 1;
		if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0) {
			ss << "Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
			logger->log(Err, ss.str());
			ss.str("");
			ss.clear();
		}

		if (addSocket(msgSocket, RECEIVE) == false) {
			logger->log(Info, "\t\tToo many connections, dropped!");
			closesocket(socket.getSocketID());
		}
	}

	bool WebServer::receiveMessage(Socket& socket) {
		char BUFFER[2048];
		SOCKET msgSocket = socket.getSocketID();

		int bytesRecv = recv(msgSocket, BUFFER, sizeof(BUFFER), 0);

		if (SOCKET_ERROR == bytesRecv) {
			ss << "Server: Error at recv(): " << WSAGetLastError();;
			logger->log(Err,ss.str());
			ss.str("");
			ss.clear();
			return false;
		}

		if (bytesRecv == 0) {
			return false;
		}

		else {
			BUFFER[bytesRecv] = '\0'; //add the null-terminating to make it a string
			ss << "Server: Recieved: " << bytesRecv << " bytes of \"" << BUFFER << "\" message.";
			logger->log(Info, ss.str());
			ss.str("");
			ss.clear();

			socket.setSocketLastRecv();
			socket.setSocketSendState(HANDLE_REQ);
			//Request request = new Request (socket.id, socket.buffer);
			//socket.buff = 0;
			//socket.len = 0;
			//socket 

			return true;
		}
	}

	void WebServer::sendMessage(Socket* sokcet) {
		int bytesSent = 0;
		char sendBuff[255];

		SOCKET msgSocket = (*sokcet).getSocketID();
		strcpy(sendBuff, "response");

		bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
		if (SOCKET_ERROR == bytesSent) {
			cerr << "Server: Error at send(): " << WSAGetLastError() << endl;
			return;
		}

		ss << "Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.";
		logger->log(Info, ss.str());
		ss.str("");
		ss.clear();

		(*sokcet).setSocketSendState(IDLE);
	}

	void WebServer::printDisconnectSocket(const SOCKET& socket) {
		struct sockaddr_in name;
		int nameLen = sizeof(name);

		if (getpeername(socket, (struct sockaddr*)&name, &nameLen) == SOCKET_ERROR) {
			cerr << "Server: Error at getpeername(): " << WSAGetLastError() << endl;
		}

		ss << "Server: Client " << inet_ntoa(name.sin_addr) << ":" << ntohs(name.sin_port) << " has disconnected.";
		logger->log(Info, ss.str());
		ss.str("");
		ss.clear();
	}
}
