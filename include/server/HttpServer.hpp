#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "ClientTable.hpp"
#include "Config.hpp"
#include "EventLoop.hpp"

#include <iostream>
#include "Logger.hpp"
#include "Socket.hpp"
#include "helper.hpp"

class HttpServer {
   private:
	// Socket _socket;
	// ClientTable _table; // eventLoop owns these
	SocketTable _socketTable;
	Config _config;
	const std::string &_configPath;
	EventLoop _eventLoop;
	void createSockets(const ServerConfig &servConf);

   public:
	HttpServer(const std::string &path);

	void init();
	void run();
};

#endif
