#include "HttpServer.hpp"
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Logger.hpp"

HttpServer::HttpServer(const std::string &path)
	: _configPath(path), _eventLoop(_socketTable) {}

void HttpServer::createSockets(const ServerConfig &servConf) {
	for (size_t j = 0; j < servConf.listens.size(); ++j) {
		Socket *s = new Socket(servConf, servConf.listens[j]);
		s->configureSocket();
		s->configureAddress();
		s->bindSocket();
		s->startListening();
		_socketTable.add(s);
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
		createSockets(_config.servers[i]);
	}
	Logger::info("Server initialized");
}

void HttpServer::run() {
	Logger::info("Running...");
	_eventLoop.addSockets();
	_eventLoop.loop();
}
