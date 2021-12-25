#pragma once
#include "Request.h"
#include <winsock2.h>
#include "Response.h"
#include "Error.h"
#include "RequestParser.h"
#include <time.h>
#include <queue>
namespace web_server {

#define MAX_MSG_SIZE 2048

	enum RecvMod { LISTEN, RECEIVE };
	enum SendMod { IDLE, SEND };
	class Client
	{
	private:
		enum RecvMod recvMod;
		SOCKET socketID;
		time_t lastRecvTime;
		queue<Request>pendingRequests;
		queue<Response>readyResponses;

	public:
		Client(SOCKET socketID, enum RecvMod recvMod);
		~Client();

		time_t getLastRecvTime();
		SOCKET getSocketID();
		bool hasPendingRequests();
		bool hasReadyResponses();
		Error recvMsg();

		enum RecvMod getRecvMod();
		queue<Request> getPendingRequests();
		queue<Response> getReadyResponses();
		bool sendResponses();
	};
}
