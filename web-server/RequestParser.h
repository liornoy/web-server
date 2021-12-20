#pragma once
#include "Request.h"
namespace web_server {
	class RequestParser
	{
	public:
		static Request ParseRequest(string inComingMsg);
		static RequestLine extractRequestLine(string inComingMsg, int& readIndex);
		static map<string, string> extractRequestHeaderFields(string inComingMsg, int& readIndex);
		static string extractRequestBody(string inComingMsg, int& readIndex);
	};
}