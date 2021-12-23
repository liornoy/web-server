#pragma once
#include <string.h>
namespace web_server {
	class Response
	{
	private:
		int statusCode;
		string reasonPhrase;
		HTTPVersion ProtocolVersion;

	public:
		const char* toString() { return ""; }
	};
}