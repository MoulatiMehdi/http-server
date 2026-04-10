#include "Client.hpp"

Client::Client(const ServerConfig &servConf, int fd)
	: _fd(fd) /*, _servConf(servConf)  , _connected_at(time(NULL)) */ {
	(void)servConf;
}

Client::~Client() {
	if (_fd >= 0) close(_fd);
}

// /*  hardcode*/
// #include <map>
// #include <string>
// #include <vector>
//
//
//
// enum ParserState { MALFORMED = -2, INCOMPLETE = -1 };
// HttpRequest makeFakeReq() {
// 	HttpRequest req;
// 	req.method = "GET";
// 	req.path = "/index.html";
// 	req.version = "HTTP/1.1";
// 	req.headers["Host"] = "localhost:8080";
// 	req.headers["Connection"] = "keep-alive";
// 	req.headers["Content-Length"] = "0";
// 	return req;
// }
//
// class HttpParser {
//    public:
// 	int tryParse(char *buff, size_t size, HttpRequest &req) {
// 		(void)buff;
// 		(void)size;
// 		req = makeFakeReq();
// 		return INCOMPLETE;
// 	}
// };
// // TODO: make it inside client.hpp
// void queueResponse(const HttpResponse &res, std::vector<u_int8_t> &_wrbuf) {
// 	std::ostringstream head;
//
// 	head << "HTTP/1.0 " << res.status_code << " " << res.status_msg << "\r\n";
// 	for (std::map<std::string, std::string>::const_iterator it =
// 			 res.headers.begin();
// 		 it != res.headers.end(); ++it)
// 		head << it->first << ": " << it->second << "\r\n";
// 	head << "Content-Length: " << res.body.size() << "\r\n";
// 	head << "\r\n";
//
// 	std::string headStr = head.str();
// 	_wrbuf.insert(_wrbuf.end(), headStr.begin(), headStr.end());
// 	_wrbuf.insert(_wrbuf.end(), res.body.begin(), res.body.end());
// }
//
// HttpResponse makeFakeRes() {
// 	HttpResponse res;
// 	res.status_code = 200;
// 	res.status_msg = "OK";
// 	res.headers["Content-Type"] = "text/plain";
//
// 	std::string body = "Hello from webserve\n";
// 	res.body.insert(res.body.end(), body.begin(), body.end());
// 	return res;
// }

struct HttpRequest {
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::vector<u_int8_t> body;
};
struct HttpResponse {
	int status_code;
	std::string status_msg;
	std::map<std::string, std::string> headers;
	std::vector<u_int8_t> body;

	HttpResponse() : status_code(200), status_msg("OK") {}
};
struct RouterResult {
	bool cgi;
	std::string cgi_path;	// if cgi == true
	HttpResponse response;	// if cgi == false
};
// /*  hardcode*/

#define BUFF_SIZE 4096
void hardcode(RouterResult &rres, HttpResponse &res, HttpRequest &req) {
	req.method = "GET";
	req.path = "/index.html";
	req.version = "HTTP/1.1";
	req.headers["Host"] = "localhost:8080";
	req.headers["Connection"] = "keep-alive";

	std::string body = "Hello from webserve\n";
	res.status_code = 200;
	res.status_msg = "OK";
	res.headers["Content-Type"] = "text/html";
	res.body.insert(res.body.end(), body.begin(), body.end());

	rres.cgi = false;
	rres.response = res;
}
ClientStatus Client::onReadable() {
	// TCP layer (you) - onReadable()
	int n;
	char buff[BUFF_SIZE];
	n = read(_fd, buff, sizeof(buff));
	if (n == 0 || n == ERROR) return DISCONNECT;

	// int consumed = _parser.tryParse(_rbuf, _request);
	// if (consumed == -2) return serveError(400);
	// if (consumed == -1) return OK;	// incomplete, wait

	// hand off to router, get response back
	// RouterResult result = _router.handle(_request, _servConf);

	// if (result.isCgi()) return initCgi(result);	 // starts pipe/fork
	// queueResponse(result.response());
	//
	//
	//
	RouterResult rres;
	HttpResponse res;
	HttpRequest req;
	hardcode(rres, res, req);
	return WANT_WRITE;
}

ClientStatus Client::onWritable() {
	int n;

	if (hasDataToWrite()) {
		n = write(_fd, _wrbuf.data(), _wrbuf.size());
		if (n <= 0) return DISCONNECT;

		_wrbuf.erase(_wrbuf.begin(), _wrbuf.begin() + n);
		if (hasDataToWrite()) return OK;
		return DONE_WRITE;
	}
	return OK;
}

bool Client::hasDataToWrite() const { return !_wrbuf.empty(); }
