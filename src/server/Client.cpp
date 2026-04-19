#include "Client.hpp"
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include "Cgi.hpp"

Client::Client(const ServerConfig &servConf, int fd)
	: _fd(fd),
	  _cgi_pending(
		  false) /*, _servConf(servConf)  , _connected_at(time(NULL)) */ {
	(void)servConf;
}

Client::~Client() {
	if (_fd >= 0) close(_fd);
}

/* struct HttpRequest {
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::vector<u_int8_t> body; // file??
};

struct HttpResponse {
	int status_code;
	std::string status_msg;
	std::map<std::string, std::string> headers;
	std::vector<u_int8_t> body;
	std::vector<u_int8_t> body;

	HttpResponse() : status_code(200), status_msg("OK") {}
};
struct RouterResult {
	bool cgi;
	std::string cgi_path;	// if cgi == true
	HttpResponse response;	// if cgi == false
}; */

#define BUFF_SIZE 4096

#include "HttpRequest.hpp"

void readFile(const char *path, std::vector<u_int8_t> &buffer) {
	int fd = open(path, O_RDONLY);
	if (fd < 0) exit(54);

	char tmp[4096];
	ssize_t bytes;

	while ((bytes = read(fd, tmp, sizeof(tmp))) > 0) {
		buffer.insert(buffer.end(), tmp, tmp + bytes);
	}

	close(fd);
}

ClientStatus Client::queueResponse(const HttpResp &resp) {
	std::ostringstream head;

	head << "HTTP/1.0 " << to_stringg(resp.status_code) << " "
		 << resp.status_msg << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it =
			 resp.headers.begin();
		 it != resp.headers.end(); ++it)
		head << it->first << ": " << it->second << "\r\n";
	head << "\r\n";

	std::string headStr = head.str();
	_wrbuf.insert(_wrbuf.end(), headStr.begin(), headStr.end());
	// if (resp.isFile) readFile(resp.path.c_str(), _wrbuf);
	// if (resp.isFile) return initFileServe(resp.path);
	if (_file) return WANT_WRITE;
	else _wrbuf.insert(_wrbuf.end(), resp.body.begin(), resp.body.end());

	return WANT_WRITE;
}

#include <sys/stat.h>
#include "helper.hpp"

void Client::initFileServe(const std::string &path) {
	_file = new FileServe(path);
	if (_file->done()) {
		delete _file;
		_file = NULL;
	}
}

ClientStatus Client::serveFile(const std::string &path) {
	initFileServe(path);
	if (_file == NULL) { return serveErr(404); }

	HttpResp resp(200, "OK");
	resp.isFile = true;
	resp.path = path;
	resp.headers["Content-Type"] = "text/html";
	resp.headers["Content-Length"] = to_stringg(_file->size());
	resp.headers["Connection"] = "close";
	return queueResponse(resp);
}

ClientStatus Client::serveErr(int code) {
	// return queueResponse(_router.buildError(status, _servConf));
	// a response that has the filename or string
	std::string msg("Not Found");
	HttpResp resp(code, msg);

	// Body (simple HTML)
	std::string body =
		"<html>\n"
		"<head><title>" +
		to_stringg(code) + " " + msg +
		"</title></head>\n"
		"<body>\n"
		"<h1>" +
		to_stringg(code) + "From Memory" + msg +
		"</h1>\n"
		"</body>\n"
		"</html>\n";
	resp.body.assign(body.begin(), body.end());
	resp.isFile = true;
	resp.path = "./err.html";
	resp.headers["Content-Type"] = "text/html";
	_file = new FileServe(resp.path);
	if (_file->done()) {
		std::cout << "\n\n\nFile Failed to open Err page\n\n\n\n";
		delete _file;
		_file = NULL;
		resp.headers["Content-Length"] = to_stringg(resp.body.size());
	} else {
		std::cout << "\n\n\nFile open opened\n\n\n\n";
		resp.headers["Content-Length"] = to_stringg(_file->size());
	}
	resp.headers["Connection"] = "close";
	return queueResponse(resp);
}

Cgi *Client::initCgi() {
	_cgi = new Cgi("./hello.sh", _req);	 // try catch
	return _cgi;
}

bool Client::cgiPending() const { return _cgi_pending; }
Cgi *Client::getCgi() const { 

	return _cgi; }

int Client::getFd() const { return _fd; }

ClientStatus Client::onReadable() {
	char buff[BUFF_SIZE];
	int n = read(_fd, buff, sizeof(buff));
	if (n == 0 || n == ERROR) return DISCONNECT;
	// std::cout.write(buff, n); // for debug

	// return serveErr(400);
	// return serveFile("hello.html");

	_cgi_pending = true;
	return OK;
	// _parser.parse(_req, buff, n);
	// if (!_req.good()) return serveErr(_req.status());  // returns WANT_WRITE
	// if (!_req.complete()) return OK;

	// RouterResult result = _router.handle(_req, _servConf);
	// if (result.isCgi()) return initCgi(result); returns OK	 // starts
	// pipe/fork

	// return queueResponse(result.response()); // returns WANT_WRITE

	return WANT_WRITE;
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

bool Client::hasDataToWrite() const { return !_wrbuf.empty(); }
