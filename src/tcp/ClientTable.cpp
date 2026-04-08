#include "ClientTable.hpp"

ClientTable::ClientTable() {}

ClientTable::~ClientTable() { clear(); }

void ClientTable::add(int fd) { clients[fd] = new Client(fd); }

ClientMap::iterator ClientTable::remove(int fd) {
	ClientMap::iterator it = clients.find(fd);

	if (it != clients.end()) {
		ClientMap::iterator rem = it++;
		delete rem->second;
		clients.erase(rem);
		return it;
	}
	return it;
}

Client *ClientTable::get(int fd) {
	ClientMap::iterator it = clients.find(fd);

	if (it != clients.end()) return it->second;
	return NULL;
}

ClientMap &ClientTable::getAll() { return clients; }

size_t ClientTable::size() {
	return clients.size();
}

void ClientTable::clear() {
	for (ClientMap::iterator it = clients.begin(); it != clients.end(); ++it)
		delete it->second;
	clients.clear();
}
