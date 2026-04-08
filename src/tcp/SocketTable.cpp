#include "SocketTable.hpp"

SocketTable::SocketTable() {}

SocketTable::~SocketTable() { clear(); }

void SocketTable::add(Socket *socket) { _sockets.push_back(socket); }

Socket *SocketTable::operator[](int i) const { return _sockets[i]; }

int SocketTable::getSocket(int fd) {
	for (size_t i = 0; i < _sockets.size(); ++i)
		if (_sockets[i]->getFd() == fd) return i;
	return -1;
}

// SocketVec::iterator SocketTable::remove(int fd) {
// 	SocketVec::iterator it = _sockets.find(fd);
//
// 	if (it != _sockets.end()) {
// 		SocketVec::iterator rem = it++;
// 		delete rem->second;
// 		_sockets.erase(rem);
// 		return it;
// 	}
// 	return it;
// }
//
// Socket *SocketTable::get(int fd) {
// 	SocketVec::iterator it = _sockets.find(fd);
//
// 	if (it != _sockets.end()) return it->second;
// 	return NULL;
// }
//
// SocketVec &SocketTable::getAll() { return _sockets; }

size_t SocketTable::size() { return _sockets.size(); }

void SocketTable::clear() {
	for (SocketVec::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		delete *it;
	_sockets.clear();
}
