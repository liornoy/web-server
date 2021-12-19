#include "Socket.h"

namespace web_server {
	Socket::Socket(SOCKET id, int recvStatus) {
		setSocketLastRecv();
		this->id = id; socketState.send = IDLE;
		socketState.recv = recvStatus;
	}

	bool Socket::operator==(const Socket& other) const {

		return id == other.id;
	}

	void Socket::setSocketLastRecv() {
		time(&socketState.lastRecvTime); 
	}

	void Socket::setSocketSendState(int newState) { 
		socketState.send = newState;
	}

	void Socket::setInComingRequest(char* msg) {
		this->inComingRequest[0] = '\0';
		if (strlen(msg) < MAX_MSG_SIZE) {
			strcpy(inComingRequest, msg);
		}
	}
	void Socket::setOutGoingResponse(char* msg) {
		this->outgoingResponse[0] = '\0';
		if (strlen(msg) < MAX_MSG_SIZE) {
			strcpy(outgoingResponse, msg);
		}
	}

	char* Socket::getOutGoingResponse() {

		return outgoingResponse;
	}

	char* Socket::getInComingResponse() {

		return inComingRequest; 
	}
}