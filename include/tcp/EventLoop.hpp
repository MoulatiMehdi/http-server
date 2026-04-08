#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "ClientTable.hpp"
#include "Socket.hpp"

#include <fcntl.h>
#include <sys/epoll.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include "Logger.hpp"
#include "helper.hpp"
#include "SocketTable.hpp"

#define MAX_EVENTS 128
#define MAX_CLIENTS 1000

class EventLoop {
   private:
	SocketTable &_sockTable;
	ClientTable _cliTable;
	int _epollfd;

	void handleNewConnections(int sockFd);
	void processClients(struct epoll_event &ev);
	void disconnectClient(int fd);
	void epollMod(int fd, uint32_t events);
	void epollAdd(int fd, uint32_t events);
	bool handleStatus(int fd, ClientStatus status);

   public:
	EventLoop(SocketTable &_socketTable);
	~EventLoop();

	SocketTable &getSockTable();
	ClientTable &getCliTable();
	void loop();
};

#endif
