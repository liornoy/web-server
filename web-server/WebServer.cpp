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

		while (true)
		{
			numOfFD = selectSockets(&waitRecv, &waitSend);

			if (SOCKET_ERROR == numOfFD) {
				cerr << "Server: Error at select(): " << WSAGetLastError() << endl;
				WSACleanup();
				return;
			}
			handleWaitRecv(numOfFD, &waitRecv);
			handleWaitSend(numOfFD, &waitSend);
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

	bool WebServer::addSocket(SOCKET id, int recvStatus)
	{
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
				FD_SET((*it).GetSocketID(), waitRecv);
			if (currSocketState.send == SEND_RESPONSE)
				FD_SET((*it).GetSocketID(), waitSend);
		}
		return select(0, waitRecv, waitSend, NULL, NULL);
	}

	void WebServer::handleWaitRecv(int& numOfFD, fd_set* waitRecv) {
		list<Socket>::iterator it;
		for (it = sockets.begin(); it != sockets.end() && numOfFD > 0; ++it) {
			if (FD_ISSET((*it).GetSocketID(), waitRecv))
			{
				numOfFD--;
				switch ((*it).getSocketState().recv)
				{
				case LISTEN:
					acceptConnection((*it));
					break;

				case RECEIVE:
					// false if connection needs to shut down
					if (false == receiveMessage(*it)) {
						it = sockets.erase(it);
						--it;
					}
					break;
				}
			}
		}
	}

	void WebServer::handleWaitSend(int& numOfFD, fd_set* waitSend) {
		list<Socket>::iterator it;
		for (it = sockets.begin(); it != sockets.end() && numOfFD > 0; ++it) {
			if (FD_ISSET((*it).GetSocketID(), waitSend))
			{
				numOfFD--;
				sendMessage(&(*it));
			}
		}
	}

	void WebServer::acceptConnection(Socket& socket)
	{
		struct sockaddr_in from;
		int fromLen = sizeof(from);

		SOCKET msgSocket = accept(socket.GetSocketID(), (struct sockaddr*)&from, &fromLen);
		if (INVALID_SOCKET == msgSocket)
		{
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
		if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
		{
			ss << "Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
			logger->log(Err, ss.str());
			ss.str("");
			ss.clear();
		}

		if (addSocket(msgSocket, RECEIVE) == false)
		{
			logger->log(Info, "\t\tToo many connections, dropped!");
			closesocket(socket.GetSocketID());
		}
		return;
	}

	bool WebServer::receiveMessage(Socket& socket) {
		char BUFFER[2048];
		SOCKET msgSocket = socket.GetSocketID();

		int bytesRecv = recv(msgSocket, BUFFER, sizeof(BUFFER), 0);

		if (SOCKET_ERROR == bytesRecv) {
			ss << "Server: Error at recv(): " << WSAGetLastError();;
			logger->log(Err,ss.str());
			ss.str("");
			ss.clear();
			printDisconnectSocket(msgSocket);
			closesocket(msgSocket);
			return false;
		}

		if (bytesRecv == 0) {
			printDisconnectSocket(msgSocket);
			closesocket(msgSocket);
			return false;
		}

		else {
			BUFFER[bytesRecv] = '\0'; //add the null-terminating to make it a string
			ss << "Server: Recieved: " << bytesRecv << " bytes of \"" << BUFFER << "\" message.";
			logger->log(Info, ss.str());
			ss.str("");
			ss.clear();

			socket.setSocketSendState(HANDLE_REQ);
			//Request request = new Request (socket.id, socket.buffer);
			//socket.buff = 0;
			//socket.len = 0;
			//socket 

			return true;
		}
	}

	void WebServer::sendMessage(Socket* sokcet)
	{
		int bytesSent = 0;
		char sendBuff[255];

		SOCKET msgSocket = (*sokcet).GetSocketID();
		strcpy(sendBuff, "response");

		bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
		if (SOCKET_ERROR == bytesSent)
		{
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
