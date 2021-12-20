#include "RequestParser.h"

namespace web_server {
	Request RequestParser::ParseRequest(string inComingMsg) {
		Request request;
		int readIndex = 0;
		request.requestLine = extractRequestLine(inComingMsg, readIndex);
		request.requestHeaderFields = extractRequestHeaderFields(inComingMsg, readIndex);
		request.body = extractRequestBody(inComingMsg, readIndex);
		return request;
	}
	RequestLine RequestParser::extractRequestLine(string inComingMsg, int& readIndex) {
		RequestLine requestLine;
		// enter logic...

		return requestLine;
	}
	map<string, string>RequestParser::extractRequestHeaderFields(string inComingMsg, int& readIndex) {
		map<string, string> requestHeaderFields;
		// enter logic...

		return requestHeaderFields;
	}
	string RequestParser::extractRequestBody(string inComingMsg, int& readIndex) {
		string body;
		// enter logic...

		return body;
	}
}