#include "RequestParser.h"

namespace web_server {
	Request RequestParser::ParseRequest(string inComingMsg,SOCKET socketID) {
		Request request;
		istringstream iss(inComingMsg);
		request.requestLine = extractRequestLine(inComingMsg, iss);
		request.requestHeaderFields = extractRequestHeaderFields(inComingMsg, iss);
		request.body = extractRequestBody(inComingMsg, iss);
		request.socketID = socketID;
		return request;
	}
	void setMethod(RequestLine& rl, string method) {
		if (method.compare("GET") == 0) {
			rl.methodToken = GET_REQ;
			return;
		}if (method.compare("OPTIONS") == 0) {
			rl.methodToken = OPTIONS_REQ;
			return;
		}
		if (method.compare("HEAD") == 0) {
			rl.methodToken = HEAD_REQ;
			return;
		}if (method.compare("POST") == 0) {
			rl.methodToken = POST_REQ;
			return;
		}if (method.compare("PUT") == 0) {
			rl.methodToken = PUT_REQ;
			return;
		}if (method.compare("DELETE") == 0) {
			rl.methodToken = DELETE_REQ;
			return;
		}if (method.compare("TRACE") == 0) {
			rl.methodToken = TRACE_REQ;
			return;
		}
	}

	RequestLine RequestParser::extractRequestLine(string inComingMsg, istringstream& iss) {
		RequestLine requestLine;
		
		string method,ver;
		iss >> method;
		setMethod(requestLine, method);
		iss >> requestLine.requestUri;
		iss >> ver;
		if (ver.compare(HTTPVerDot1) == 0) {
			requestLine.protocolVersion == HTTPDOT1;
		}
		else {
			requestLine.protocolVersion == HTTPDOT0;
		}
		return requestLine;
	}
	map<string, string>RequestParser::extractRequestHeaderFields(string inComingMsg, istringstream& iss) {
		map<string, string> requestHeaderFields;
		string title, content;
		while (iss >> title) {
			if (title.back() != ':')
				break;
			iss >> content;
			cout << title << " " << content << endl;
			requestHeaderFields.insert(pair<string, string>(title, content));
		} 

		return requestHeaderFields;
	}

	string RequestParser::extractRequestBody(string inComingMsg, istringstream& iss) {
		string body, cat;
		while (iss >> cat) {
			body.append(cat);
		}
		return body;
	}
}