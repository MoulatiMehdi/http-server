#include "HttpServer.hpp"
#include <exception>
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Logger.hpp"

HttpServer::HttpServer(const std::string &path)
	: _configPath(path), _eventLoop(_socketTable) {}

void HttpServer::createSockets(const ServerConfig &servConf) {
	for (size_t j = 0; j < servConf.listens.size(); ++j) {
		Socket *s = new Socket(servConf, servConf.listens[j]);
		try {
			s->configureSocket();
			s->configureAddress();
			s->bindSocket();
			s->startListening();
			_socketTable.add(s);
		} catch (const std::exception &e) {
			delete s;
			throw e;
		}
		Logger::info("Socket: [" + toString(s->getFd()) + "] Listeting on: " +
					 s->getAddr() + ":" + toString(s->getPort()));
	}
	std::cout << "----------------------------------------\n";
}

void HttpServer::init() {
	ConfigParser parser;

	_config = parser.parseFile(_configPath);
	parser.printConfig(_config);

	for (size_t i = 0; i < _config.servers.size(); ++i) {
		Logger::info("Virtual server [" + toString(i) + "]:");
		try {
			createSockets(_config.servers[i]);
		} catch (const std::exception &e) {
			Logger::error("HttpServer::init: " + std::string(e.what()));
		}
	}
	if (_socketTable.size()) Logger::info("Webserv initialized");
	else exitError("Webserv failed to initialize: no listening socket");
}

void HttpServer::run() {
	Logger::info("Running...");
	_eventLoop.addSockets();
	_eventLoop.loop();
}
