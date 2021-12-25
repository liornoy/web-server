#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <list>
#include <map>
#include <queue>
#include "Socket.h"
#include "Logger.h"
#include "RequestParser.h"
#include "Response.h"
#include <fstream>
namespace web_server {
#define CR 13
#define LF 10

	class Response
	{
	public:
		int statusCode;
		string reasonPhrase;
		HTTPVersion ProtocolVersion;
		string body;
		map<string, string> headers;
		string toString();
	};
}
