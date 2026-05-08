#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <map>

#include <fcntl.h>

#include "ClientTable.hpp"
#include "Socket.hpp"
#include "SocketTable.hpp"

#define MAX_EVENTS 128
#define MAX_CLIENTS 1000
#define CLI_TIMEOUT_MS 1000
#define EPOLL_TIMEOUT_MS 1000
#define CGI_TIMEOUT_MS 5000

class EventLoop {
   public:
	EventLoop(SocketTable &socketTable);
	~EventLoop();

   private:
	void epollAdd(int fd, uint32_t events);
	void epollMod(int fd, uint32_t events);

	void disconnectClient(const Client *cli);

	int handleStatus(Client *client, ClientStatus status);

	void processClients(struct epoll_event &ev);
	void processCgi(struct epoll_event &ev);

	void handleNewConnections(Socket *sock);

	void disconnectTimedOut(const std::vector<Client *> &clients);
	void handleCgiTimeout(Client *client);
	bool cgiTimedOut(Client *client, time_t now);
	bool clientTimedOut(Client *client, time_t now);
	void runMaintenance();

	void registerCgiPipes(const Client *client);

   public:
	void addSockets();
	void remSockets();
	void loop();

   private:
	SocketTable &_sockTable;
	ClientTable _cliTable;
	int _epollfd;

	std::map<int, int> _pipe_to_client;
};

#endif
