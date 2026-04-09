#ifndef SOCKTTABLE_HPP
#define SOCKTTABLE_HPP

#include <vector>
#include "Socket.hpp"
#include "Config.hpp"

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
	size_t size();
	void clear();
};

#endif
