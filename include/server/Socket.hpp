#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "Config.hpp"
#include "helper.hpp"

#define QUEUE_SIZE 10

class Socket {
   private:
	const ServerConfig &_servConf;
	const ListenConfig &_listenConf;
	int _fd;
	int _port;
	struct sockaddr_in _addr;
	// ref to Config

   public:
	Socket(const ServerConfig &servConf, const ListenConfig &listen);
	Socket(const Socket &);
	Socket &operator=(const Socket &);
	~Socket();

	void configureSocket();
	void configureAddress();
	void bindSocket();
	void startListening();
	int acceptClient();
	const ServerConfig &getServConf();
	int getFd();
	std::string getAddr();
	int getPort();
};
#endif
