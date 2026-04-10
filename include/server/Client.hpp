#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "Config.hpp"
#include "Logger.hpp"
#include "helper.hpp"

enum ClientStatus { OK, WANT_WRITE, DONE_WRITE, DISCONNECT };

class Client {
   private:
	int _fd;
	// const ServerConfig &_servConf;
	std::vector<u_int8_t> _wrbuf;

	// time_t _connected_at;
	// time_t _last_response_at;
	// bool _request_complete;
	bool hasDataToWrite() const;

   public:
	Client(const ServerConfig &servConf, int fd);
	~Client();

	ClientStatus onReadable();
	ClientStatus onWritable();
};
