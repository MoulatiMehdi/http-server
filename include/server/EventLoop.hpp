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
#include "SocketTable.hpp"
#include "helper.hpp"

#define MAX_EVENTS 128
#define MAX_CLIENTS 1000

class EventLoop {
   private:
	SocketTable &_sockTable;
	ClientTable _cliTable;
	int _epollfd;
	std::map<int, int> _pipe_to_client;

	void handleNewConnections(Socket *sock);
	void processClients(struct epoll_event &ev);
	void disconnectClient(int fd);
	void epollMod(int fd, uint32_t events);
	void epollAdd(int fd, uint32_t events);
	bool handleStatus(Client *client, ClientStatus status);

   public:
	EventLoop(SocketTable &_socketTable);
	~EventLoop();

	void addSockets();
	void loop();
};

#endif
