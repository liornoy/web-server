#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
namespace web_server {
	const int LISTEN = 0;
	const int RECEIVE = 1;
	const int IDLE = 2;
	const int HANDLE_REQ = 3;
	const int PROC_REQUEST = 4;
	const int SEND_RESPONSE = 5;

	struct SocketState
	{
		int recv;			
		int	send;			// Sending?
	};

	class Socket
	{
	private:
		SOCKET id;
		SocketState socketState;
		char buffer[128];

	public:
		Socket(SOCKET id, int recvStatus); 
		const SOCKET GetSocketID()const { return id; }
		const SocketState getSocketState()const { return socketState; }
		void setSocketSendState(int newState){ socketState.send = newState; }
		bool operator==(const Socket& other) const;
	};
}