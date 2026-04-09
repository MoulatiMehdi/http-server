#include "ClientTable.hpp"

ClientTable::ClientTable() {}

ClientTable::~ClientTable() { clear(); }

void ClientTable::add(const ServerConfig &servConf, int fd) {
	_clients[fd] = new Client(servConf, fd);
}

ClientMap::iterator ClientTable::remove(int fd) {
	ClientMap::iterator it = _clients.find(fd);

	if (it != _clients.end()) {
		ClientMap::iterator rem = it++;
		delete rem->second;
		_clients.erase(rem);
		return it;
	}
	return it;
}

Client *ClientTable::get(int fd) {
	ClientMap::iterator it = _clients.find(fd);

	if (it != _clients.end()) return it->second;
	return NULL;
}

size_t ClientTable::size() { return _clients.size(); }

void ClientTable::clear() {
	for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
}
