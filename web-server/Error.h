#pragma once
#include <iostream>
#include <string.h>
using namespace std;
namespace web_server{
#define ERR_MSG_LEN 75
	class Error
	{
	private:
		void* err = nullptr;
		char errMsg[ERR_MSG_LEN];
		Error() = delete;

	public:
		Error(const char* errorDescription);
		Error(const Error&);
		friend bool operator!=(Error const& l, Error const& r);
		friend ostream& operator<<(ostream& os, const Error& err);
	};
}
