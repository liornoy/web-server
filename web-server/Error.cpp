#include "Error.h"

namespace web_server {
	ostream& operator<<(ostream& os, const Error& err) {
		os << err.errMsg;
		return os;
	}

	Error::Error(const char* errorDescription) {
		if (strlen(errorDescription) < ERR_MSG_LEN) {
			strcpy_s(errMsg, errorDescription);
			err = &errMsg;
		}
	}
	Error::Error(const Error& other) {
		strcpy_s(errMsg, other.errMsg);
		err = &errMsg;
	}
	bool operator!=(Error const& l, Error const& r) {
		return l.err != r.err;
	}
}