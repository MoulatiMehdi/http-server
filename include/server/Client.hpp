#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>

#include "Cgi.hpp"
#include "Config.hpp"
#include "FileServe.hpp"
#include "HttpRequest.hpp"
#include "RouteResult.hpp"
#include "Status.hpp"

enum ClientStatus { OK, WANT_WRITE, DONE_WRITE, DISCONNECT, INIT_CGI };

class Client {
   public:
	Client(const ServerConfig &servConf, int fd);
	~Client();

   private:
	ClientStatus handleRoute(const RouteResult &routeResult);
	ClientStatus initCgi(const std::string &path);

   public:
	void queueResponse(const std::string &raw);

	void serveFile(const std::string &path, status::Status code,
				   const std::string &type);
	void serveDir(const std::string &path);
	void serveErr(status::Status status);


	ClientStatus onCgiDone();
	ClientStatus onReadable();
	ClientStatus onWritable();

	bool cgiPending() const;
	Cgi *getCgi() const;
	int getFd() const;
	time_t connectedAt() const;

   private:
	int _fd;
	const ServerConfig &_servConf;

	std::vector<u_int8_t> _wrbuf;
	HttpRequest _req;

	FileServe *_file;
	Cgi *_cgi;

	time_t _connected_at;
};

#endif
