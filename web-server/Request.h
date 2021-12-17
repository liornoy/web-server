#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <map>
#include <iostream>

using namespace std;
namespace web_server {
	enum MethodType_enum { GET_REQ, OPTIONS_REQ, HEAD_REQ, POST_REQ, PUT_REQ, DELETE_REQ, TRACE_REQ };

	typedef struct RequestHeader {
		MethodType_enum methodType;
		string requestUri;
		map<string, string>requestHeaderFields;
	}RequestHeader;

	typedef struct Request {
		RequestHeader header;
		string body;
	}Request;
}