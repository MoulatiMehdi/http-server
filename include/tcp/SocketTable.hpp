#ifndef SOCKTTABLE_HPP
#define SOCKTTABLE_HPP

#include <vector>
#include "Socket.hpp"

typedef std::vector<Socket *> SocketVec;

class SocketTable {
   private:
	SocketVec _sockets;

   public:
	SocketTable();
	~SocketTable();

	void add(Socket *socket);
	Socket *operator[](int i) const;
	int getSocket(int fd);

	// remove ??
	// SocketVec::iterator remove(int fd);
	// SocketVec &getAll();
	// Socket *get(int fd);
	size_t size();
	void clear();
};

#endif
