#pragma once
#include <winsock2.h>
#include "Request.h"
#include <sstream>
namespace web_server {
	class RequestParser
	{
	public:
		static Request ParseRequest(string inComingMsg, SOCKET socketID);
		
	private:
		static RequestLine extractRequestLine(string inComingMsg, istringstream& iss);
		static map<string, string> extractRequestHeaderFields(string inComingMsg, istringstream& iss);
		static string extractRequestBody(string inComingMsg, istringstream& iss);
	};
}