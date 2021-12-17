#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <list>

const int SERVER_PORT = 27015;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int HANDLE_REQ = 4;
const int PROC_REQUEST = 1;
const int SEND_RESPONSE = 2;

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	char buffer[128];
	int len;

public: 
	bool operator==(const SocketState& other) const;
};

list<SocketState>sockets;



class WebServer
{
public:
	void Run();
private:
	SOCKET listenSocket;
	void closeServer();
	bool addSocket(SOCKET id, int what);
	void acceptConnection(SOCKET id);
	bool receiveMessage(SocketState* socket);
	void sendMessage(SocketState* socket_ptr);
	void initListenSocket();
	void printDisconnectSocket(SOCKET* socket);
};

