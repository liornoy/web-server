#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <time.h>

namespace web_server {
	const int LISTEN = 0;
	const int RECEIVE = 1;
	const int IDLE = 2;
	const int HANDLE_REQ = 3;
	const int SEND_RESPONSE = 4;
	const int MAX_MSG_SIZE = 2048;
	struct SocketState
	{
		int recv;			
		int	send;			// Sending?
		time_t lastRecvTime;
	};

	class Socket
	{
	private:
		SOCKET id;
		SocketState socketState;
		char inComingRequest[MAX_MSG_SIZE], outgoingResponse[MAX_MSG_SIZE];

	public:
		Socket(SOCKET id, int recvStatus); 
		const SOCKET getSocketID()const { return id; }
		const SocketState getSocketState()const { return socketState; }
		void setSocketSendState(int newState);
		void setSocketLastRecv();
		bool operator==(const Socket& other) const;
		char* getOutGoingResponse();
		char* getInComingResponse();

		
		void setInComingRequest(char* msg);
	};
}