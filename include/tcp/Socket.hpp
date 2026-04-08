#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "helper.hpp"

#define QUEUE_SIZE 10

class Socket {
   private:
	int _fd;
	int _port;
	struct sockaddr_in _addr;
	// ref to Config

   public:
	Socket(int port);
	Socket(const Socket &);
	Socket &operator=(const Socket &);
	~Socket();

	void configureSocket();
	void configureAddress();
	void bindSocket();
	void startListening();
	int acceptClient();
	int getFd();
	int getPort();
};
#endif
