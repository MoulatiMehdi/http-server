#include "Socket.hpp"

Socket::Socket(int port) : _port(port) {
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0) exitError("socket");
}

Socket::~Socket() {
	if (_fd != -1) close(_fd);
}
void make_non_blocking(int fd);
void Socket::configureSocket() {
	int opt = 1;

	make_non_blocking(_fd);
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
				   sizeof(opt)))
		exitError("setsocket");
}

void Socket::configureAddress() {
	std::memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = INADDR_ANY;
}

void Socket::bindSocket() {
	if (bind(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
		exitError("bind");
}

void Socket::startListening() {
	if (listen(_fd, QUEUE_SIZE) < 0) exitError("listen");
}

// int Socket::acceptClient() {
// 	struct sockaddr_in client_addr;
// 	socklen_t len = sizeof(client_addr);
//
// 	int client_fd = accept(_fd, (struct sockaddr *)&client_addr, &len);
//
// 	if (client_fd < 0) return -1;
// 	return client_fd;
// }
//
int Socket::getFd() { return _fd; }
int Socket::getPort() { return _port; }
