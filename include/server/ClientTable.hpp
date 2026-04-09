#ifndef CLIENTTABLE_HPP
#define CLIENTTABLE_HPP

#include <map>
#include "Client.hpp"
#include "Config.hpp"

typedef std::map<int, Client *> ClientMap;

class ClientTable {
   private:
	ClientMap _clients;

   public:
	ClientTable();
	~ClientTable();

	void add(const ServerConfig &servConf, int fd);
	ClientMap::iterator remove(int fd);
	// ClientMap &getAll();
	Client *get(int fd);
	size_t size();
	void clear();
};

#endif
