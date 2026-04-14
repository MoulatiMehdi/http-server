#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "Config.hpp"
#include "HttpParser.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "helper.hpp"

struct HttpResp {
	int status_code;
	std::string status_msg;
	std::map<std::string, std::string> headers;
	bool isFile;
	std::string path;
	std::vector<u_int8_t> body;

	HttpResp(int n, const std::string &s) : status_code(n), status_msg(s) {}
};

enum ClientStatus { OK, WANT_WRITE, DONE_WRITE, DISCONNECT };

class Client {
   private:
	int _fd;
	// const ServerConfig &_servConf;
	std::vector<u_int8_t> _wrbuf;
	HttpParser _parser;
	HttpRequest _req;

	// time_t _connected_at;
	// time_t _last_response_at;
	// bool _request_complete;
	bool hasDataToWrite() const;

   public:
	Client(const ServerConfig &servConf, int fd);
	~Client();

	ClientStatus onReadable();
	ClientStatus onWritable();

	ClientStatus serveErr(int status);
	ClientStatus queueResponse(const HttpResp &resp);
};

// while the request isnt complete          => parser required
// while the request isnt served to the cgi => request required
