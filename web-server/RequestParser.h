#pragma once
#include "Request.h"
namespace web_server {
	class RequestParser
	{
	public:
		static Request ParseRequest(string request);
	};

}