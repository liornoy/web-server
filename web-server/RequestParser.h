#pragma once
#include "Request.h"
namespace web_server {
	class RequestParser
	{
	public:
		struct Request ParseRequest(string request);
	};

}