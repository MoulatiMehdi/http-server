#include "EventLoop.hpp"
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include "Cgi.hpp"
#include "Client.hpp"
#include "ClientTable.hpp"
#include "Logger.hpp"
#include "Status.hpp"
#include "helper.hpp"

// #include <sys/epoll.h>
// #include <iostream>
//
void printEpollEvents(uint32_t events) {
	if (events & EPOLLIN) std::cout << "EPOLLIN ";
	if (events & EPOLLOUT) std::cout << "EPOLLOUT ";
	if (events & EPOLLERR) std::cout << "EPOLLERR ";
	if (events & EPOLLHUP) std::cout << "EPOLLHUP ";
	if (events & EPOLLRDHUP) std::cout << "EPOLLRDHUP ";
	if (events & EPOLLET) std::cout << "EPOLLET ";
	if (events & EPOLLONESHOT) std::cout << "EPOLLONESHOT ";

	std::cout << std::endl;
}

EventLoop::EventLoop(SocketTable &_socketTable) : _sockTable(_socketTable) {
	_epollfd = epoll_create(1);
	if (_epollfd == ERROR) exitError("epoll_create");
	std::cout << _socketTable.size() << std::endl;
}

EventLoop::~EventLoop() { close(_epollfd); }

void EventLoop::epollAdd(int fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_ADD");
}

void EventLoop::epollMod(int fd, u_int32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_MOD");
	printEpollEvents(ev.events);
}

void EventLoop::disconnectClient(const Client *cli) {
	int cliFd = cli->getFd();
	if (cli->getCgi()) {
		int cgiIn = cli->getCgi()->getIn();
		int cgiOut = cli->getCgi()->getOut();
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, cgiIn, NULL);
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, cgiOut, NULL);
		_pipe_to_client.erase(cgiIn);
		_pipe_to_client.erase(cgiOut);
	}
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, cliFd, NULL) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_DEL");
	_cliTable.remove(cliFd);
	Logger::info("Client " + toString(cliFd) + ": disconnected");
}

int EventLoop::handleStatus(Client *client, ClientStatus status) {
	int fd = client->getFd();

	if (status == DISCONNECT) return -1;
	else if (status == WANT_WRITE) epollMod(fd, EPOLLOUT);
	else if (status == DONE_WRITE) disconnectClient(client);
	else if (status == INIT_CGI) registerCgiPipes(client);
	return 0;
}

void EventLoop::processClients(struct epoll_event &ev) {
	Client *client = _cliTable.get(ev.data.fd);
	ClientStatus status = OK;

	printEpollEvents(ev.events);
	if (ev.events & (EPOLLERR | EPOLLHUP)) {
		disconnectClient(client);
		return;
	}
	if (ev.events & EPOLLIN) {
		status = client->onReadable();
	} else if (ev.events & EPOLLOUT) {
		status = client->onWritable();
	}

	if (handleStatus(client, status) == -1) disconnectClient(client);
	return;
}

void EventLoop::processCgi(struct epoll_event &ev) {
	int cgiFd = ev.data.fd;
	int clientFd = _pipe_to_client[cgiFd];

	Client *client = _cliTable.get(clientFd);
	if (!client) {
		_pipe_to_client.erase(cgiFd);
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, cgiFd, NULL);
		close(cgiFd);
		return;
	}

	Cgi *cgi = client->getCgi();
	if (!cgi) return;

	CgiStatus status = CGI_OK;
	int cgiOut = cgi->getOut();

	if (ev.events & EPOLLIN || ev.events & EPOLLHUP) status = cgi->onReadable();
	else if (ev.events & EPOLLOUT) status = cgi->onWritable();

	if (status == CGI_DONE || status == CGI_ERROR) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, cgiFd, NULL);
		_pipe_to_client.erase(cgiFd);

		if (cgiFd == cgiOut) handleStatus(client, client->onCgiDone());
	}
}

void EventLoop::handleNewConnections(Socket *sock) {
	int cliFd;
	struct sockaddr_in cliAddr;
	socklen_t len = sizeof(cliAddr);
	const ServerConfig &servConf = sock->getServConf();

	while (true) {
		cliFd = accept(sock->getFd(), (struct sockaddr *)&cliAddr, &len);
		if (cliFd == -1) return;

		makeNonBlocking(cliFd);
		_cliTable.add(servConf, cliFd);
		epollAdd(cliFd, EPOLLIN);
		Logger::info("New Client: " + std::string(inet_ntoa(cliAddr.sin_addr)) +
					 ":" + toString(ntohs(cliAddr.sin_port)) + " through " +
					 sock->getAddr() + ":" + toString(sock->getPort()));
	}
}
void EventLoop::disconnectTimedOut(const std::vector<Client *> &clients) {
	for (size_t i = 0; i < clients.size(); ++i) {
		Logger::info("Client " + toString(clients[i]->getFd()) + ": timed out");
		disconnectClient(clients[i]);
	}
}

void EventLoop::handleCgiTimeout(Client *client) {
	Logger::warn("CGI timeout for client " + toString(client->getFd()));
	client->serveErr(status::GATEWAY_TIMEOUT);
	handleStatus(client, WANT_WRITE);
}

bool EventLoop::cgiTimedOut(Client *client, time_t now) {
	time_t passedSec = now - client->getCgi()->startedAt();
	return client->cgiPending() && (passedSec * 1000 > CGI_TIMEOUT_MS);
}

bool EventLoop::clientTimedOut(Client *client, time_t now) {
	return (now - client->connectedAt()) * 1000 > CLI_TIMEOUT_MS;
}

void EventLoop::runMaintenance() {
	time_t now = time(NULL);
	std::vector<Client *> toDisconnect;

	const ClientMap &cli = _cliTable.getAll();
	for (ClientMap::const_iterator it = cli.begin(); it != cli.end(); ++it) {
		Client *client = it->second;
		if (clientTimedOut(client, now)) toDisconnect.push_back(client);
		else if (client->cgiPending() && cgiTimedOut(client, now))
			handleCgiTimeout(client);
	}
	disconnectTimedOut(toDisconnect);
}

void EventLoop::registerCgiPipes(const Client *client) {
	Cgi *cgi = client->getCgi();
	int clientFd = client->getFd();

	int in = cgi->getIn();
	int out = cgi->getOut();

	epollAdd(in, EPOLLOUT);
	epollAdd(out, EPOLLIN);

	_pipe_to_client[in] = clientFd;
	_pipe_to_client[out] = clientFd;
}

void EventLoop::addSockets() {
	for (size_t i = 0; i < _sockTable.size(); ++i)
		epollAdd(_sockTable[i]->getFd(), EPOLLIN);
}

void EventLoop::loop() {
	// TODO: maxfd
	int nfds;
	struct epoll_event events[MAX_EVENTS];
	int sockIndex;

	while (true) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, EPOLL_TIMEOUT_MS);
		if (nfds == ERROR) exitError("epoll_wait");
		if (nfds == 0) { runMaintenance(); }
		for (int n = 0; n < nfds; ++n) {
			sockIndex = _sockTable.getSocket(events[n].data.fd);
			if (sockIndex >= 0) handleNewConnections(_sockTable[sockIndex]);
			else if (_cliTable.get(events[n].data.fd) != NULL)
				processClients(events[n]);
			else if (_pipe_to_client.find(events[n].data.fd) !=
					 _pipe_to_client.end())
				processCgi(events[n]);
		}
	}
}
