#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <map>
#include <iostream>
#include <winsock2.h>
using namespace std;
namespace web_server {
	enum MethodType { GET_REQ, OPTIONS_REQ, HEAD_REQ, POST_REQ, PUT_REQ, DELETE_REQ, TRACE_REQ };
	enum HTTPVersion { HTTPDOT0, HTTPDOT1 };
	const string HTTPVerDot1 = "HTTP/1.1";
	const string HTTPVetDot0 = "HTTP/1.0";

	typedef struct RequestLine {
		MethodType methodToken;
		string requestUri;
		HTTPVersion protocolVersion;
	}RequestLine;

	typedef struct Request {
		SOCKET socketID;
		RequestLine requestLine;
		map<string, string>requestHeaderFields;
		string body;
	}Request;
}