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
}