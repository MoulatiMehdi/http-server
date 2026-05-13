
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "HttpServer.hpp"

int main(int ac, char **av) {
    (void)ac;
	HttpServer server(av[1] ? av[1] : "");

	signal(SIGPIPE, SIG_IGN);
	// try catch
	server.init();
	server.run();
	return 0;
}
