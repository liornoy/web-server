#pragma once
#include "Request.h"
#include "Response.h"
namespace web_server {
	class ResponseCreator
	{
	private:
		map<int, string>statusCodes = {
		{100, "Continue"},
		{200, "OK"},
		{201, "Created"},
		{202, "Accepted"},
		{204, "No Content"},
		{400, "Bad Request"},
		{404, "Not Found"},
		{406, "Not Acceptable"},
		};
	public:
		static Response CreateResponse(Request request);
	};
}