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
#include <queue>
#include "Logger.h"
#include "RequestParser.h"
#include "Response.h"
#include "Client.h"
#include "RequestHandler.h"
using namespace std;

namespace web_server {
	class WebServer
	{
	public:
		void run();
		WebServer(int maxSockets, int serverPort);
		WebServer(WebServer const&) = delete;
		void operator=(WebServer const&) = delete;

	private:
		const int TIME_OUT = 120;
		const string SERVER_FILES_PATH = "C:/temp";
		const timeval SELECT_TIME_OUT_VAL{ TIME_OUT,0 };
		list<Client>clients;
		Logger* logger;
		int maxClients, serverPort;
		list<list<Client>::iterator> clientsToDelete;

		int selectSockets(fd_set* waitRecv, fd_set* waitSend);
		bool addNewClient(SOCKET id, int recvStatus);
		void acceptConnection(Socket& socket);
		bool receiveMessage(Socket& socket);
		void sendMessage(Socket* socket);
		SOCKET initListenSocket();
		void printDisconnectSocket(const SOCKET& socket);
		void handleWaitRecv(list<Socket>::iterator& socketIterator, fd_set* waitRecv, int& numOfFD);
		void handleWaitSend(list<Socket>::iterator& socketIterator, fd_set* waitSend, int& numOfFD);
		void handleTimeOut(list<Socket>::iterator& socketIterator, time_t now);
		void handleInComingRequests(list<Socket>::iterator& socketIterator);
		void handleClients(int numOfFD, fd_set* waitRecv, fd_set* waitSend);
		void deleteSockets();
	};
}
