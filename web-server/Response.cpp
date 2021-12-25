#include "Response.h"
namespace web_server {
	string Response::toString() {
		this->headers["Content-Type"] = "text/html";
		string res;
		if (this->ProtocolVersion == HTTPDOT0)
			res.append(HTTPVetDot0);
		else
			res.append(HTTPVerDot1);
		res.append(" ");
		res.append(to_string(this->statusCode));
		res.append(" ");
		res.append(this->reasonPhrase);
		res.push_back(CR);
		res.push_back(LF);
		auto itr = headers.begin();
		while (itr != headers.end()) {
			res.append(itr->first);
			res.append(": ");
			res.append(itr->second);
			res.push_back(CR);
			res.push_back(LF);
			itr++;
		}
		res.push_back(CR);
		res.push_back(LF);
		res.append(body);
		if (body.length() == 0) {
			res.push_back(CR);
			res.push_back(LF);
		}
		return res;
	}
}