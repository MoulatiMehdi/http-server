#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "Cgi.hpp"
#include "Config.hpp"
#include "FileServe.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "RouteResult.hpp"
#include "Status.hpp"
#include "helper.hpp"

enum ClientStatus { OK, WANT_WRITE, DONE_WRITE, DISCONNECT, INIT_CGI };

// choose one naming convention
class Client {
   private:
	int _fd;
	const ServerConfig &_servConf;
	std::vector<u_int8_t> _wrbuf;
	HttpRequest _req;

	FileServe *_file;
	Cgi *_cgi;

	// time_t _connected_at;
	// time_t _last_response_at;
	// bool _request_complete;

   public:
	Client(const ServerConfig &servConf, int fd);
	~Client();

	ClientStatus onReadable();
	ClientStatus onWritable();
	ClientStatus onCgiDone();
	ClientStatus initCgi(const std::string &path);
	bool cgiPending() const;
	Cgi *getCgi() const;
	int getFd() const;

	ClientStatus handleRoute(const RouteResult &routeResult);  // TODO: private
	void serveFile(const std::string &path, status::Status code,
				   const std::string &type);
	void serveDir(const std::string &path);
	void serveErr(status::Status status);
	void queueResponse(const std::string &raw);
	// void queueResponse(const HttpResponse &resp, const std::string &body);
};

// while the request isnt complete          => parser required
// while the request isnt served to the cgi => request required
