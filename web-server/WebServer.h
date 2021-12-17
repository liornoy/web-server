#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <list>
#include <map>
#include "Socket.h"

using namespace std;

namespace web_server {

	const int SERVER_PORT = 27015;
	const int MAX_SOCKETS = 60;

	class WebServer
	{
	public:
		void Run();
	private:
		list<Socket>sockets;

		int selectSockets(fd_set* waitRecv, fd_set* waitSend);
		bool addSocket(SOCKET id, int recvStatus);
		void acceptConnection(Socket* socket);
		bool receiveMessage(Socket* socket);
		void sendMessage(Socket* socket);
		SOCKET initListenSocket();
		void printDisconnectSocket(SOCKET* socket);
		void handleWaitRecv(int& numOfFD,fd_set* waitRecv);
		void handleWaitSend(int& numOfFD,fd_set* waitSend);
	};

}