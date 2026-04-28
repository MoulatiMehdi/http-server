#ifndef CLIENTTABLE_HPP
#define CLIENTTABLE_HPP

#include <map>
#include "Client.hpp"
#include "Config.hpp"
#define MAX_FD FD_SETSIZE

typedef std::map<int, Client *> ClientMap;

class ClientTable {
   private:
	   // Client *_clientFdTable[MAX_FD];
	ClientMap _clients;

   public:
	ClientTable();
	~ClientTable();

	void add(const ServerConfig &servConf, int fd);
	ClientMap::iterator remove(int fd);
	const ClientMap &getAll();
	Client *get(int fd);
	size_t size();
	void clear();
};

#endif
