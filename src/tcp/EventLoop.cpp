#include "EventLoop.hpp"

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

bool EventLoop::handleStatus(int fd, ClientStatus status) {
	if (status == DISCONNECT) return false;
	else if (status == WANT_WRITE) epollMod(fd, EPOLLIN | EPOLLOUT);
	else if (status == DONE_WRITE) epollMod(fd, EPOLLIN);
	return true;
}

void EventLoop::processClients(struct epoll_event &ev) {
	int fd = ev.data.fd;
	Client *client = _cliTable.get(fd);

	if (ev.events & EPOLLIN && !handleStatus(fd, client->onReadable())) {
		disconnectClient(fd);
		return;
	}
	if (ev.events & EPOLLOUT && !handleStatus(fd, client->onWritable())) {
		disconnectClient(fd);
		return;
	}
}

void EventLoop::handleNewConnections(Socket *sock) {
	int cliFd;
	struct sockaddr_in cliAddr;
	socklen_t len = sizeof(cliAddr);

	while (true) {
		cliFd = accept(sock->getFd(), (struct sockaddr *)&cliAddr, &len);
		if (cliFd == -1) return;

		make_non_blocking(cliFd);
		_cliTable.add(cliFd);
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

// TODO: ihajji: 9lab 3la kifach maxfd
void EventLoop::loop() {
	int nfds;
	struct epoll_event events[MAX_EVENTS];
	int sockIndex;

	std::cout << "size: " << _socketTable.size() << "\n";
	while (true) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, ERROR /* time out */);
		if (nfds == ERROR) exitError("epoll_wait");
		for (int n = 0; n < nfds; ++n) {
			sockIndex = _sockTable.getSocket(events[n].data.fd);
			if (sockIndex >= 0)
				handleNewConnections(_sockTable[sockIndex]);
			else processClients(events[n]);
		}
	}
}
