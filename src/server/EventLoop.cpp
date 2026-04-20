#include "EventLoop.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include "Cgi.hpp"
#include "ClientTable.hpp"

EventLoop::EventLoop(SocketTable &_socketTable) : _sockTable(_socketTable) {
	_epollfd = epoll_create1(0);
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
}

void EventLoop::disconnectClient(int fd) {
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_DEL");
	_cliTable.remove(fd);
	Logger::info("client " + to_stringg(fd) + ": disconnected");
}

bool EventLoop::handleStatus(Client *client, ClientStatus status) {
	int fd = client->getFd();

	if (status == DISCONNECT) return false;
	else if (status == WANT_WRITE) epollMod(fd, EPOLLIN | EPOLLOUT);
	else if (status == DONE_WRITE) epollMod(fd, EPOLLIN);
	else if (status == INIT_CGI) registerCgiPipes(client);
	return true;
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

	if (ev.events & EPOLLIN) status = cgi->onReadable();
	else if (ev.events & EPOLLOUT) status = cgi->onWritable();

	if (status == CGI_DONE || status == CGI_ERROR) {
		epoll_ctl(_epollfd, EPOLL_CTL_DEL, cgiFd, NULL);
		_pipe_to_client.erase(cgiFd);

		if (status == CGI_ERROR) disconnectClient(clientFd);
		else if (status == CGI_DONE && cgiFd == cgi->getOut())
			client->onCgiDone();
	}
}

void EventLoop::processClients(struct epoll_event &ev) {
	int fd = ev.data.fd;
	Client *client = _cliTable.get(fd);

	if (ev.events & EPOLLIN && !handleStatus(client, client->onReadable())) {
		disconnectClient(fd);
		return;
	}
	if (ev.events & EPOLLOUT && !handleStatus(client, client->onWritable())) {
		disconnectClient(fd);
		return;
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

		make_non_blocking(cliFd);
		_cliTable.add(servConf, cliFd);
		epollAdd(cliFd, EPOLLIN);
		Logger::info("Client: " + std::string(inet_ntoa(cliAddr.sin_addr)) +
					 ":" + to_stringg(ntohs(cliAddr.sin_port)) + " through " +
					 sock->getAddr() + ":" + to_stringg(sock->getPort()));
	}
}

void EventLoop::addSockets() {
	for (size_t i = 0; i < _sockTable.size(); ++i)
		epollAdd(_sockTable[i]->getFd(), EPOLLIN);
}

// TODO: maxfd
void EventLoop::loop() {
	int nfds;
	struct epoll_event events[MAX_EVENTS];
	int sockIndex;

	while (true) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, ERROR /* time out */);
		if (nfds == ERROR) exitError("epoll_wait");
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
