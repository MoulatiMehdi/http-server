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
#include "SessionManager.hpp"
#include "Status.hpp"

enum ClientStatus { OK, WANT_WRITE, DONE_WRITE, DISCONNECT, INIT_CGI };

class Client {
   public:
	Client(const ServerConfig &servConf, int fd, SessionManager &sessions);
	~Client();

   private:
	ClientStatus handleRoute(const RouteResult &routeResult);
	ClientStatus initCgi(const std::string &path);

   public:
	void queueResponse(const std::string &raw);
	void queueResponse(HttpResponse &resp);
	void resolveSession();
	void finalizeResponse(HttpResponse &resp);
	void serveSessionDemo();

	void serveFile(const std::string &path, status::Status code,
				   const std::string &type);
	void serveDir(const std::string &path);
	void serveErr(status::Status status);
	void serveRedir(const std::string &path, status::Status code);

	ClientStatus onCgiDone();
	ClientStatus onReadable();
	ClientStatus onWritable();

	bool cgiPending() const;
	Cgi *getCgi() const;
	int getFd() const;
	time_t lastActivity() const;

   private:
	int _fd;
	const ServerConfig &_servConf;

	std::vector<u_int8_t> _wrbuf;
	HttpRequest _req;

	FileServe *_file;
	Cgi *_cgi;

	time_t _last_activity;
	SessionManager &_sessions;
	bool _newSession;
	std::string _sid;
	SessionData *_session;
};

#endif
