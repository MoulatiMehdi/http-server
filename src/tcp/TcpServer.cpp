#include "TcpServer.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"

TcpServer::TcpServer(const std::string &path) : _configPath(path), _eventLoop(_socketTable) {}

void TcpServer::init() {
	ConfigParser parser;
	Config config;

	config = parser.parseFile(_configPath);
	// if (config.bad())
	// 	throw; // ??
	for (size_t i = 0; i < config.servers.size(); ++i) {
		Socket *s = new Socket(config.servers[i].listen_port);
		s->configureSocket();
		s->configureAddress();
		s->bindSocket();
		s->startListening();
		_socketTable.add(s);
		Logger::info("Server: [" + to_stringg(s->getFd()) + "] Listeting on: " 
					 /* + to_stringg(s->getAddr()) TODO: implement*/
					 + to_stringg(s->getPort()));
	}
	Logger::info("Server initialized");
}

void TcpServer::run() {
	Logger::info("Server running");
	_eventLoop.loop();
}
