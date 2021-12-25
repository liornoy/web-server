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
				logger->log(Err, "Server: Error at select(): " + to_string(WSAGetLastError()));
				WSACleanup();
				return;
			}
			handleSockets(numOfFD, &waitRecv, &waitSend);
		}

		logger->log(Info, "Server: Server Shutting Down.");
		closesocket(listenSocketId);
		WSACleanup();
	}
	void putDate(Response& resp) {
		char buf[1000];
		time_t now = time(0);
		struct tm tm = *gmtime(&now);
		strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S", &tm);
		resp.headers["Date"] = buf;
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
		handleInComingRequests();
		deleteSockets();
	}

	void WebServer::handleInComingRequests() {
		while (!pendingRequests.empty()) {
			Request req = pendingRequests.front();
			pendingRequests.pop();
			string response = handleReq(req);
			for (auto socketIterator = sockets.begin(); socketIterator != sockets.end(); ++socketIterator) {
				if (socketIterator->getSocketID() == req.socketID) {
					socketIterator->setSocketSendState(SEND_RESPONSE);
					socketIterator->setResponse(response.c_str());
				}
			}
		}
	}
	string WebServer::handleReq(Request req) {
		switch (req.requestLine.methodToken) {
		case GET_REQ:
			return Get(req);

		case OPTIONS_REQ:
			return Options(req);

		case HEAD_REQ:
			return Head(req);

		case POST_REQ:
			return Post(req);

		case PUT_REQ:
			return Put(req);

		case DELETE_REQ:
			return Delete(req);

		case TRACE_REQ:
			return Trace(req);
		}
		return "bad request";
	}
	string WebServer::Get(Request req) {
		string url = req.requestLine.requestUri;
		if (url.find_first_of('?') != string::npos){
			int save;
			string lang;
			int queryIndex = url.find_first_of('?') + 1;
			save = queryIndex;
			if (url.substr(queryIndex, 4).compare("lang") != 0) {
				return "this server supports only lang query params";
			}
			queryIndex += 5;
			if (url.substr(queryIndex, 2).compare("he") == 0)
				lang = "/he";
			else if (url.substr(queryIndex, 2).compare("en") == 0)
				lang = "/en";
			else if (url.substr(queryIndex, 2).compare("fr") == 0)
				lang = "/fr";
			else
				return "this servers suports only he/fr/en languages";

			url = url.substr(0, save-1);
			lang = lang.append(url);
			url = lang;
		}
		
		string finalURL = server_dir;
		finalURL.append(url);
		Response response;
		putDate(response);
		ifstream fileStream(finalURL);

		if (fileStream.is_open()) {
			response.reasonPhrase = "OK";
			response.statusCode = 200;
			

			char c;
			int index = 0;

			while (!fileStream.eof())
			{
				fileStream.get(c); 
				response.body.push_back(c);
				index++;
			}
			response.headers.insert(pair<string,string>("Content-Length", to_string(index)));
			fileStream.close();
		}
		else {
			response.reasonPhrase = "NOT FOUND";
			response.statusCode = 404;
			response.headers.insert(pair<string, string>("Content-Length", "0"));
		}
		return response.toString();
	}

	string WebServer::Options(Request req) {
		Response response;
		response.headers["Allow"] = "GET HEAD POST OPTIONS TRACE PUT DELETE";
		response.headers["Content-Length"] = "0";
		response.statusCode = 204;
		putDate(response);
		response.reasonPhrase = "No Content";
		return response.toString();
	}

	string WebServer::Head(Request req) {
		string url = req.requestLine.requestUri;
		if (url.find_first_of('?') != string::npos) {
			int save;
			string lang;
			int queryIndex = url.find_first_of('?') + 1;
			save = queryIndex;
			if (url.substr(queryIndex, 4).compare("lang") != 0) {
				return "this server supports only lang query params";
			}
			queryIndex += 5;
			if (url.substr(queryIndex, 2).compare("he") == 0)
				lang = "/he";
			else if (url.substr(queryIndex, 2).compare("en") == 0)
				lang = "/en";
			else if (url.substr(queryIndex, 2).compare("fr") == 0)
				lang = "/fr";
			else
				return "this servers suports only he/fr/en languages";

			url = url.substr(0, save - 1);
			lang = lang.append(url);
			url = lang;
		}

		string finalURL = server_dir;
		finalURL.append(url);
		Response response;
		ifstream fileStream(finalURL);
		putDate(response);

		if (fileStream.is_open()) {
			response.reasonPhrase = "OK";
			response.statusCode = 200;
		}
		else {
			response.reasonPhrase = "NOT FOUND";
			response.statusCode = 404;
			response.headers.insert(pair<string, string>("Content-Length", "0"));
		}
		return response.toString();
	}
	string WebServer::Post(Request req) {
		Response response;
		response.headers["Content-Length"] = "0";
		putDate(response);

		cout << "Server got post request: " << req.body << endl;

		if (req.requestLine.requestUri == "") {
			response.statusCode = 200;
			response.reasonPhrase = "OK";
		}
		else {
			response.statusCode = 400;
			response.reasonPhrase = "Bad Request";
		}
		return response.toString();

	}
	string WebServer::Delete(Request req) {
		Response response;
		putDate(response);
		char numStr[10];
		string finalURL = server_dir;
		finalURL.append(req.requestLine.requestUri);


		ifstream fileStream(finalURL);

		if (!fileStream)
		{
			response.statusCode = 404;
			response.reasonPhrase = "Not Found";
			return response.toString();
		}
		fileStream.close();

		if (remove(finalURL.data()) == 0)
		{
			response.statusCode = 204;
			response.reasonPhrase = "No Content";
		}
		else
		{
			response.statusCode = 202;
			response.reasonPhrase = "Accepted";
			response.body = "Fail DELETE action has not yet been performed";
			response.headers["Content - Length"] = to_string(response.body.length());
		}
	}

	string WebServer::Trace(Request req) {
		Response response;
		putDate(response);

		
		response.headers["Content - Type"] = "message/http";;
		
		response.body = req.body;
		response.headers["Content - Length"] = to_string(response.body.length());

		response.statusCode = 202;
		response.reasonPhrase = "OK";

		return response.toString();
	}

	string WebServer::Put(Request req) {
		Response response;
		char numStr[10];
		string finalURL = server_dir;
		finalURL.append(req.requestLine.requestUri);

		ifstream file(finalURL);

		if (!file) 
		{
			ofstream newFile(finalURL); 
			if (!newFile) 
			{
				response.statusCode = 500;
				response.reasonPhrase = "Internal Server Error";
				response.headers["Content-Length"] = "0";
				return response.toString();
			}
			
			newFile << req.body;

			response.statusCode = 201;
			response.reasonPhrase = "Created";
			response.headers["Content-Length"] = "0";
			return response.toString();
		}
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

	void WebServer::handleWaitSend(list<Socket>::iterator& socketIterator, fd_set* waitSend, int& numOfFD) {
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
			cout << "bybyb" << endl;
			return false;
		}

		else {
			buffer[bytesRecv] = '\0'; //add the null-terminating to make it a string
			logger->log(Info, "Server: Recieved: " + bytesRecv + string(" bytes of \"") + buffer + "\" message.");

			socket.setSocketLastRecv();
			pendingRequests.push(RequestParser::ParseRequest(buffer, msgSocket));

			return true;
		}
	}

	void WebServer::sendMessage(Socket* sokcetPtr) {
		int bytesSent = 0;

		SOCKET msgSocket = (*sokcetPtr).getSocketID();

		bytesSent = send(msgSocket, sokcetPtr->getOutGoingResponse(), (int)strlen(sokcetPtr->getOutGoingResponse()), 0);
		if (SOCKET_ERROR == bytesSent) {
			logger->log(Err, "Server: Error at send(): " + to_string(WSAGetLastError()));
			return;
		}

		logger->log(Info, "Server: Sent: " + bytesSent + string("\\") + to_string(strlen(sokcetPtr->getOutGoingResponse())) + \
			" bytes of \"" + string(sokcetPtr->getOutGoingResponse()) + "\" message.");

		(*sokcetPtr).setSocketSendState(IDLE);
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