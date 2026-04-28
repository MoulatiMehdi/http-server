#include "Socket.hpp"
#include <sstream>

Socket::Socket(const ServerConfig &servConf, const ListenConfig &listenConf)
	: _servConf(servConf),
	  _listenConf(listenConf),
	  _port(listenConf.port),
	  _ip(listenConf.host) {
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
#include <arpa/inet.h>
#include <netinet/in.h>

std::string ipToString(in_addr_t addr) {
	char buffer[INET_ADDRSTRLEN];

	if (inet_ntop(AF_INET, &addr, buffer, INET_ADDRSTRLEN) == NULL)
		throw std::runtime_error("inet_ntop failed");

	return std::string(buffer);
}

int strToIp(const std::string &ipstr) {
	int bytes[4];
	char c;
	std::stringstream ss(ipstr);
	ss >> bytes[0];
	ss >> c;
	ss >> bytes[1];
	ss >> c;
	ss >> bytes[2];
	ss >> c;
	ss >> bytes[3];

	return (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3];
}

void Socket::configureAddress() {
	std::memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);

	_addr.sin_addr.s_addr = htonl(strToIp(_ip));
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
