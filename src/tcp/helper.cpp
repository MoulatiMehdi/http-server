#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include "Logger.hpp"

// NOTE: helper


void make_non_blocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << std::strerror(errno) << std::endl;
		std::cerr << "fcntl() failed\n";
		return;
	}

	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void exitError(std::string msg) {
	Logger::error(msg + ": " + std::strerror(errno));
	std::exit(EXIT_FAILURE);
}
