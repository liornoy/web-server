#include "Client.h"
namespace web_server {
	Client::Client(SOCKET socketID, enum RecvMod recvMod){
		this->recvMod = recvMod;
		this->socketID = socketID;
		time(&this->lastRecvTime);
	}

	// getLastRecvTime returns the lastRecvTime member.
	time_t Client::getLastRecvTime() {

		return lastRecvTime;
	}

	// sendResponses sends all the responses that are ready to be sent back.
	// returns true if an error accured.
	bool Client::sendResponses(){
		bool err = false;

		while (!readyResponses.empty() && err == false) {
			Response response = readyResponses.front();
			readyResponses.pop();
			const char* msg = response.toString();
			int bytesSent = send(socketID, msg, (int)strlen(msg), 0);
			if (SOCKET_ERROR == bytesSent) {
				err = true;
			}
		}

		return err;
	}

	// hasPendingRequests returns true if there are requests pending.
	bool Client::hasPendingRequests() {
		
		return pendingRequests.size() > 0;
	}

	// hasReadyResponses returs true if there are responses ready.
	bool Client::hasReadyResponses() {

		return readyResponses.size() > 0;
	}

	// getRecvMod returns the recvMod member. 
	enum RecvMod Client::getRecvMod() {

		return recvMod;
	}

	// getPendingRequests returns the pendingRequets member.
	queue<Request> Client::getPendingRequests() {

		return pendingRequests;
	}

	// getReadyResponses returns the readyResponses member.
	queue<Response> Client::getReadyResponses() {

		return readyResponses;
	}
	
	Client::~Client() {
		closesocket(socketID);
	}

	// getSocketID returns the socketID member.
	SOCKET Client::getSocketID() {
		
		return socketID; 
	}
}
