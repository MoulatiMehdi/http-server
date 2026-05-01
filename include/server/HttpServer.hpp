#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "Config.hpp"
#include "EventLoop.hpp"


class HttpServer {
   private:
	SocketTable _socketTable;
	Config _config;
	const std::string _configPath;
	EventLoop _eventLoop;

	void createSockets(const ServerConfig &servConf);

   public:
	HttpServer(const std::string &path);

	void init();
	void run();
};

#endif
