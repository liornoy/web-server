#include "WebServer.h"

void main()
{
	WebServer server;
	server.Run();
}
void WebServer::Run(){

	// Init Server's Listen Socket
	initListenSocket();
	
	while (true)
	{
		fd_set waitRecv;
		FD_ZERO(&waitRecv);

		fd_set waitSend;
		FD_ZERO(&waitSend);

		std::list<SocketState>::iterator it;

		for (it = sockets.begin(); it != sockets.end(); ++it) {
			if (((*it).recv == LISTEN) || ((*it).recv == RECEIVE))
				FD_SET((*it).id, &waitRecv);
			if ((*it).send == SEND_RESPONSE)
				FD_SET((*it).id, &waitSend);
		}

		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);

		if (nfd == SOCKET_ERROR)
		{
			cerr << "Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (it = sockets.begin(); it != sockets.end() && nfd > 0; ++it) {
			if (FD_ISSET((*it).id, &waitRecv))
			{
				nfd--;
				switch ((*it).recv)
				{
				case LISTEN:
					acceptConnection((*it).id);
					break;

				case RECEIVE:
					if (!receiveMessage(&(*it))) {
						it = sockets.erase(it);
						--it;
					}
					break;
				}
			}
		}
		for (it = sockets.begin(); it != sockets.end() && nfd > 0; ++it) {
			if (FD_ISSET((*it).id, &waitSend))
			{
				nfd--;
				sendMessage(&(*it));
			}
		}
	}

	closeServer();
}

void WebServer::closeServer() {
	cout << "Server: Server Shutting Down.\n";
	closesocket(listenSocket);
	WSACleanup();
}

void WebServer::initListenSocket() {
	WSAData wsaData;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cerr << "Server: Error at WSAStartup()\n";
		return;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == listenSocket)
	{
		cerr << "Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	sockaddr_in serverService;

	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_family = AF_INET;
	serverService.sin_port = htons(SERVER_PORT);

	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cerr << "Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cerr << "Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	cout << "Server is ready and running on address " << inet_ntoa(serverService.sin_addr) << ":" << SERVER_PORT << endl;

	addSocket(listenSocket, LISTEN);
}

void WebServer::printDisconnectSocket(SOCKET* socket) {
	struct sockaddr_in name;
	int nameLen = sizeof(name);

	if (getpeername(*socket,(struct sockaddr*) & name, &nameLen) == SOCKET_ERROR) {
		cerr << "Server: Error at getpeername(): " << WSAGetLastError() << endl;
	}
	cout << "Server: Client " << inet_ntoa(name.sin_addr) << ":" << ntohs(name.sin_port) << " has disconnected." << endl;
}

bool WebServer::addSocket(SOCKET id, int what)
{
	if (sockets.size() < MAX_SOCKETS) {
		SocketState socket;

		socket.id = id;
		socket.recv = what;
		socket.send = IDLE;
		socket.len = 0;

		sockets.push_back(socket);
		return (true);
	}
	return (false);
}


void WebServer::acceptConnection(SOCKET id)
{
	struct sockaddr_in from;
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cerr << "Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	//
	// Set the socket to be in non-blocking mode.
	//
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cerr << "Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

bool WebServer::receiveMessage(SocketState* socket)
{
	SOCKET msgSocket = (*socket).id;

	int len = (*socket).len;
	int bytesRecv = recv(msgSocket,&(*socket).buffer[len], sizeof((*socket).buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cerr << "Server: Error at recv(): " << WSAGetLastError() << endl;
		printDisconnectSocket(&msgSocket);
		closesocket(msgSocket);
		return false;
	}

	if (bytesRecv == 0)
	{
		printDisconnectSocket(&msgSocket);
		closesocket(msgSocket);
		return false;
	}

	else
	{
		(*socket).buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Server: Recieved: " << bytesRecv << " bytes of \"" << &(*socket).buffer[len] << "\" message.\n";

		(*socket).len += bytesRecv;

		if ((*socket).len > 0)
		{
			(*socket).send = HANDLE_REQ;
			//Request request = new Request (socket.id, socket.buffer);
			//socket.buff = 0;
			//socket.len = 0;
			//socket 
		}
		return true;
	}

}

void WebServer::sendMessage(SocketState* socket_ptr)
{
	int bytesSent = 0;
	char sendBuff[255];

	SOCKET msgSocket = (*socket_ptr).id;
	strcpy(sendBuff, "response");

	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cerr << "Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	(*socket_ptr).send = IDLE;
}

bool SocketState::operator==(const SocketState& other) const {
	return id == other.id;
}