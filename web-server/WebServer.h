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
#include "Logger.h"
#include<sstream>

using namespace std;



namespace web_server {
	class WebServer
	{
	public:
		void run();
		WebServer(int maxSockets, int serverPort);
		WebServer( WebServer const&) = delete;
		void operator=(WebServer const&) = delete;

	private:
		const int TIME_OUT = 120;
		const timeval SELECT_TIME_OUT_VAL{ TIME_OUT,0 };
		list<Socket>sockets;
		Logger* logger;
		int maxSockets, serverPort;
		std::stringstream ss; 

		int selectSockets(fd_set* waitRecv, fd_set* waitSend);
		bool addSocket(SOCKET id, int recvStatus);
		void acceptConnection(Socket& socket);
		bool receiveMessage(Socket& socket);
		void sendMessage(Socket* socket);
		SOCKET initListenSocket();
		void printDisconnectSocket(const SOCKET& socket);
		void handleWaitRecv(int& numOfFD,fd_set* waitRecv);
		void handleWaitSend(int& numOfFD,fd_set* waitSend);
		void handleTimeOut();
	};
}
