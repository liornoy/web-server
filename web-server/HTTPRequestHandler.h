#pragma once
#include "Request.h"
namespace web_server {
	typedef struct Response {
		StatusLine statusLine;
	}Response;

	typedef struct StatusLine {
		HTTPVersion protocolVersion;
		string statusCode; //TODO - make enum
	}StatusLine;

	class HTTPRequestHandler
	{
	public:
		static char* HandleRequest(Request req);
	};
}
