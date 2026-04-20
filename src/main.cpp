
#include <csignal>
#include <iostream>
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "HttpServer.hpp"

int main(int ac, char **av) {
	if (ac != 2) {
		std::cout << "Invalide arguments" << std::endl;
		std::cout << "  Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	HttpServer server(av[1]);  // TODO: rename to HttpServer

	signal(SIGPIPE, SIG_IGN);
	// try catch
	server.init();
	server.run();
	return 0;
}
