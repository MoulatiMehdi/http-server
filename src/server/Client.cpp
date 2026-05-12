#include "Client.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <queue>
#include <string>
#include "Cgi.hpp"
#include "FileServe.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "RouteResult.hpp"
#include "Router.hpp"
#include "Status.hpp"
#include "helper.hpp"

Client::Client(const ServerConfig &servConf, int fd, SessionManager &sessions)
	: _fd(fd),
	  _servConf(servConf),
	  _req(servConf),
	  _file(NULL),
	  _cgi(NULL),
	  _headers_complete(false),
	  _started_at(time(NULL)),
	  _last_activity(time(NULL)),
	  _sessions(sessions),
	  _newSession(false) {}

Client::~Client() {
	if (_fd >= 0) close(_fd);
	if (_file) delete _file;
	if (_cgi) delete _cgi;
}

void Client::queueResponse(const std::string &raw) {
	_wrbuf.clear();
	_wrbuf.insert(_wrbuf.end(), raw.begin(), raw.end());
}

void Client::queueResponse(HttpResponse &resp) {
	_wrbuf.clear();
	if (_file) resp.setContentLength(_file->size());
	std::string raw = resp.to_string();
	Logger::info("\n\033[1;90m" + raw + "\033[0m\n");
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

	HttpResponse resp(code);
	resp.setHeader("Content-type", type);
	finalizeResponse(resp);
	return queueResponse(resp);
}

void Client::serveDir(const std::string &path) {
	HttpResponse resp;

	if (!resp.good()) return serveErr(resp.status());
	finalizeResponse(resp);
	return queueResponse(resp.serve_directory(path, ""));
}

void Client::serveErr(status::Status code) {
	std::string errPath = _servConf.errorPage(code);

	if (!errPath.empty()) {
		try {
			_file = new FileServe(errPath);
			HttpResponse resp(code);
			resp.setHeader("Content-Type", "text/html");
			resp.setHeader("Connection", "close");
			return queueResponse(resp);
		} catch (const std::exception &e) {
			Logger::error(std::string("serveErr: ") + e.what());
		}
	}

	HttpResponse resp(code);
	finalizeResponse(resp);
	queueResponse(resp.serve_page());
}

void Client::serveRedir(const std::string &path, status::Status code) {
	HttpResponse resp(code);
	resp.setHeader("Location", path);
	finalizeResponse(resp);
	queueResponse(resp.to_string());
}

void Client::finalizeResponse(HttpResponse &resp) {
	if (_newSession)
		resp.setHeader("Set-Cookie", "sid=" + _sid + "; Path=/; HttpOnly");
	resp.setHeader("Connection", "close");
}

void Client::resolveSession() {
	_newSession = false;
	_session = NULL;
	_sid = _req.extract_key("Cookie", "sid");

	if (!_sid.empty()) _session = _sessions.get(_sid);

	if (!_session) {
		_sid = _sessions.create();
		_session = _sessions.get(_sid);
		_newSession = true;
	}

	int visits = std::atoi((*_session)["visits"].c_str());
	(*_session)["visits"] = toString(visits + 1);
	(*_session)["last_page"] = _req.uri().path();
}

ClientStatus Client::handleRoute(const RouteResult &res) {
	if (_req.uri().path() == "/session") return serveSessionDemo(), WANT_WRITE;
	std::cerr << "path:::  " << _req.uri().path() << "\n";
	switch (res.action) {
		case ROUTE_STATIC_FILE:
			return serveFile(res.path, res.statusCode, res.type), WANT_WRITE;
		case ROUTE_DIRECTORY_LISTING:
			return serveDir(res.path), WANT_WRITE;
		case ROUTE_ERROR:
			return serveErr(res.statusCode), WANT_WRITE;
		case ROUTE_CGI:
			return initCgi(res.cmd, res.path), INIT_CGI;
		case ROUTE_REDIRECT:
			return serveRedir(res.path, res.statusCode), WANT_WRITE;
		case ROUTE_DELETE:
			return queueResponse(HttpResponse(res.statusCode).to_string()),
				   WANT_WRITE;
		case ROUTE_UPLOAD:
			HttpResponse resp(res.statusCode);
			resp.setHeader("Location", "/");
			return queueResponse(resp.serve_page()), WANT_WRITE;
	}
}

void Client::serveSessionDemo() {
	if (!_session) return serveErr(status::INTERNAL_SERVER_ERROR);

	std::string visits = (*_session)["visits"];
	std::string last_page = (*_session)["last_page"];
	std::string last_upload = (*_session)["last_upload"];
	std::string created_at = (*_session)["created_at"];

	std::string body =
		"<!DOCTYPE html><html><body style='font-family:monospace;padding:40px'>"
		"<h2>Session Demo</h2>"
		"<table border='1' cellpadding='8'>"
		"<tr><td>Session ID</td><td>" +
		_sid +
		"</td></tr>"
		"<tr><td>Visits</td><td>" +
		visits +
		"</td></tr>"
		"<tr><td>Last page</td><td>" +
		last_page +
		"</td></tr>"
		"<tr><td>Last upload</td><td>" +
		(last_upload.empty() ? "(none yet)" : last_upload) +
		"</td></tr>"
		"<tr><td>Created at</td><td>" +
		created_at +
		"</td></tr>"
		"</table>"
		"<br><a href='/session/'>refresh</a> | "
		"<a href='/'>home</a> | "
		"<a href='/listing/'>listing</a>"
		"</body></html>";

	HttpResponse resp(status::OK);
	resp.setHeader("Content-Type", "text/html");
	resp.setContentLength(body.size());
	finalizeResponse(resp);
	return queueResponse(resp.to_string() + body);
}

ClientStatus Client::initCgi(const std::string &cmd, const std::string &path) {
	try {
		_cgi = new Cgi(cmd, path, _req);
	} catch (const std::exception &e) {
		Logger::error(std::string("Cgi: ") + e.what());
		return serveErr(status::INTERNAL_SERVER_ERROR), WANT_WRITE;
	}
	return INIT_CGI;
}

ClientStatus Client::onCgiDone() {
	HttpResponse resp = _cgi->getResponse();
	if (!resp.good()) return serveErr(status::BAD_GATEWAY), WANT_WRITE;
	try {
		_file = new FileServe(resp.body().c_path());
	} catch (const std::exception &e) {
		Logger::error(std::string("FileServe: ") + e.what());
		return serveErr(status::INTERNAL_SERVER_ERROR), WANT_WRITE;
	}
	queueResponse(resp);
	delete _cgi;
	_cgi = NULL;
	return WANT_WRITE;
}

ClientStatus Client::onReadable() {
	char buff[BUFF_SIZE];
	int n = read(_fd, buff, sizeof(buff));
	if (n <= 0) return DISCONNECT;

	_last_activity = std::time(NULL);
	_req.parse(buff, n);
	if (!_req.good()) return serveErr(_req.status()), WANT_WRITE;
	if (_req.body().size()) _headers_complete = true;
	if (!_req.complete()) return OK;

	resolveSession();
	return handleRoute(_req.parser().route);
}

ClientStatus Client::onWritable() {
	int n;

	if (!_wrbuf.empty()) {
		n = write(_fd, _wrbuf.data(), _wrbuf.size());
		if (n <= 0) return DISCONNECT;

		_last_activity = std::time(NULL);
		_wrbuf.erase(_wrbuf.begin(), _wrbuf.begin() + n);
		return OK;
	}

	if (_file) {
		if (_file->sendChunk(_fd) == ERROR) return DISCONNECT;
		_last_activity = std::time(NULL);
		if (_file->done()) {
			delete _file;
			_file = NULL;
			return DONE_WRITE;
		}
		return OK;
	}
	return DONE_WRITE;
}

time_t Client::lastActivity() const { return _last_activity; }
time_t Client::startedAt() const { return _started_at; }

bool Client::headersComplete() const { return _headers_complete; }
int Client::getFd() const { return _fd; }
Cgi *Client::getCgi() const { return _cgi; }
std::string Client::getRequestUri() const {
	return to_string(_req.method()) + " " + _req.uri().origin();
}
bool Client::cgiPending() const { return _cgi != NULL; }
