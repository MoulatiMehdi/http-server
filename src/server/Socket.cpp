#include "Socket.hpp"

Socket::Socket(const ServerConfig &servConf, const ListenConfig &listenConf)
	: _servConf(servConf), _listenConf(listenConf), _port(listenConf.port) {
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

const ServerConfig &Socket::getServConf() { return _servConf; }
int Socket::getFd() { return _fd; }
std::string Socket::getAddr() { return std::string(inet_ntoa(_addr.sin_addr)); }
int Socket::getPort() { return _port; }
