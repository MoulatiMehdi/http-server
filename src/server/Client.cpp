#include "Client.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include "Cgi.hpp"
#include "FileServe.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "RouteResult.hpp"
#include "Router.hpp"
#include "Status.hpp"
#include "helper.hpp"

Client::Client(const ServerConfig &servConf, int fd)
	: _fd(fd),
	  _servConf(servConf),
	  _req(servConf),
	  _file(NULL),
	  _cgi(NULL),
	  _connected_at(time(NULL)) {
	(void)servConf;
}

Client::~Client() {
	if (_fd >= 0) close(_fd);
	if (_file) delete _file;
	if (_cgi) delete _cgi;
}


void Client::queueResponse(const std::string &raw) {
	_wrbuf.clear();
	_wrbuf.insert(_wrbuf.end(), raw.begin(), raw.end());
}

void Client::serveFile(const std::string &path, status::Status code,
					   const std::string &type) {
	try {
		_file = new FileServe(path);
	} catch (const std::exception &e) {
		Logger::error(e.what());
		return serveErr(status::INTERNAL_SERVER_ERROR);
	}

	HttpResponse resp;
	resp.setStatus(code);
	resp.setContentLength(_file->size());
	resp.setHeader("Content-type", type);
	return queueResponse(resp.to_string());
}

void Client::serveDir(const std::string &path) {
	HttpResponse resp;
	std::string raw = resp.serve_directory(path, "");

	if (!resp.good()) return serveErr(resp.status());
	return queueResponse(raw);
}

void Client::serveErr(status::Status code) {
	std::string errPath = _servConf.errorPage(code);
	std::cout << "PATH:::" << errPath << "\n\n\n";

	if (!errPath.empty()) {
		try {
			_file = new FileServe(errPath);
			HttpResponse resp(code);
			resp.setHeader("Content-Type", "text/html");
			// resp.setHeader("Content-Length", to_stringg(_file->size()));
			resp.setContentLength(_file->size());
			resp.setHeader("Connection", "close");
			std::cout << resp.to_string();
			return queueResponse(resp.to_string());
		} catch (...) {}  // fall through to built-in
	}

	HttpResponse resp(code);
	std::string raw = resp.serve_page();

	return queueResponse(raw);
}

ClientStatus Client::handleRoute(const RouteResult &route) {
	Router::printRouteResult(route);
	switch (route.action) {
		case ROUTE_STATIC_FILE:
			return serveFile(route.path, route.statusCode, route.type),
				   WANT_WRITE;
		case ROUTE_DIRECTORY_LISTING:
			return serveDir(route.path), WANT_WRITE;
		case ROUTE_ERROR:
			return serveErr(route.statusCode), WANT_WRITE;
		case ROUTE_CGI:
			return initCgi(route.path), INIT_CGI;
		case ROUTE_UPLOAD:
			return queueResponse(HttpResponse(status::CREATED).to_string()),
				   WANT_WRITE;
	}
}

ClientStatus Client::initCgi(const std::string &path) {
	try {
		_cgi = new Cgi(path, _req);
	} catch (...) {
		return serveErr(status::INTERNAL_SERVER_ERROR), WANT_WRITE;
	}  // log Err
	return INIT_CGI;
}

bool flag = false;
ClientStatus Client::onCgiDone() {
	HttpResponse resp = _cgi->getResponse();
	_file = new FileServe(resp.body().c_path());
	queueResponse(_cgi->getResponse().to_string());
	flag = true; // DBG
	delete _cgi;
	_cgi = NULL;
	return WANT_WRITE;
}

ClientStatus Client::onReadable() {
	char buff[BUFF_SIZE];
	int n = read(_fd, buff, sizeof(buff));
	if (n <= 0) return DISCONNECT;

	if (flag)
		return WANT_WRITE;
	return initCgi("./cgimock2.sh");
	_req.parse(buff, n);
	if (!_req.good()) return serveErr(_req.status()), WANT_WRITE;
	if (!_req.complete()) return OK;

	return handleRoute(Router::resolve(_servConf, _req));
}

ClientStatus Client::onWritable() {
	int n;

	if (!_wrbuf.empty()) {
		n = write(_fd, _wrbuf.data(), _wrbuf.size());
		if (n <= 0) return DISCONNECT;

		_wrbuf.erase(_wrbuf.begin(), _wrbuf.begin() + n);
		return OK;
	}

	if (_file) {
		if (_file->sendChunk(_fd) == ERROR) return DISCONNECT;
		if (_file->done()) {
			delete _file;
			_file = NULL;
			return DONE_WRITE;
		}
		return OK;
	}
	return DONE_WRITE;
}

time_t Client::connectedAt() const { return _connected_at; }
int Client::getFd() const { return _fd; }
Cgi *Client::getCgi() const { return _cgi; }
bool Client::cgiPending() const { return _cgi != NULL; }
