#include "WebServer.h"

const int SERVER_PORT = 27015;
const int MAX_SOCKETS = 60;

int main() {
	web_server::WebServer server(MAX_SOCKETS, SERVER_PORT);
	server.run();
	return 0;
}