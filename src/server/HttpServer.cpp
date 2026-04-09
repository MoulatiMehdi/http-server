#include "HttpServer.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"

HttpServer::HttpServer(const std::string &path)
	: _configPath(path), _eventLoop(_socketTable) {}

void HttpServer::init() {
	ConfigParser parser;

	_config = parser.parseFile(_configPath);
	// if (config.bad())
	// 	throw; // ??
	for (size_t i = 0; i < _config.servers.size(); ++i) {
		Socket *s = new Socket(_config.servers[i]);
		s->configureSocket();
		s->configureAddress();
		s->bindSocket();
		s->startListening();
		_socketTable.add(s);
		Logger::info("Server: [" + to_stringg(s->getFd()) + "] Listeting on: " +
					 s->getAddr() + ":" + to_stringg(s->getPort()));
	}
	Logger::info("Server initialized");
}

void HttpServer::run() {
	Logger::info("Server running");
	_eventLoop.addSockets();
	_eventLoop.loop();
}
