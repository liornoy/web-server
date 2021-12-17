#include "Socket.h"

namespace web_server {
	bool Socket::operator==(const Socket& other) const {
		return id == other.id;
	}
}