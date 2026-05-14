#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <map>

#include <fcntl.h>

#include "ClientTable.hpp"
#include "SessionManager.hpp"
#include "Socket.hpp"
#include "SocketTable.hpp"

#define MAX_EVENTS 1000
#define MAX_CLIENTS 1000
#define EPOLL_TIMEOUT_S 5 // 3s
#define CLI_ACTIVITY_TIMEOUT_S 10 // 10s
#define CGI_ACTIVITY_TIMEOUT_MS 10 // 10s
#define CLI_IDLE_TIMEOUT_S 120								   //
// #define CGI_REQUEST_TIMEOUT_MS 60000 // 30s

class EventLoop {
   public:
	EventLoop(SocketTable &socketTable);
	~EventLoop();

   private:
	void epollAdd(int fd, uint32_t events);
	void epollMod(int fd, uint32_t events);

	ClientMap::iterator disconnectClient(const Client *cli);

	int handleStatus(Client *client, ClientStatus status);

	void processClients(struct epoll_event &ev);
	void processCgi(struct epoll_event &ev);

	void handleNewConnections(Socket *sock);

	void disconnectTimedOut(const std::vector<Client *> &clients);
	void handleCgiTimeout(Client *client);
	bool cgiTimedOut(Client *client);
	bool clientTimedOut(Client *client);
	void runMaintenance();

	void registerCgiPipes(const Client *client);

   public:
	void addSockets();
	void remSockets();
	void handleEvent(struct epoll_event &ev);
	void loop();

   private:
	SocketTable &_sockTable;
	ClientTable _cliTable;
	SessionManager _sessions;

	int _epollfd;

	std::map<int, int> _pipe_to_client;
	bool _connectionLimit;
};

#endif
