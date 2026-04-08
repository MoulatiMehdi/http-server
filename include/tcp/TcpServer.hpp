#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include "ClientTable.hpp"
#include "EventLoop.hpp"

#include <iostream>
#include "Logger.hpp"
#include "Socket.hpp"
#include "helper.hpp"

class TcpServer {
   private:
	// Socket _socket;
	// ClientTable _table; // eventLoop owns these
	SocketTable _socketTable;
	const std::string &_configPath;
	EventLoop _eventLoop;

   public:
	TcpServer(const std::string &path);

	void init();
	void run();
};

#endif
