#include "Client.hpp"

Client::Client(int fd) : fd(fd), _connected_at(time(NULL)) {}

Client::~Client() {
	if (fd >= 0) close(fd);
}

/*  hardcode*/
#include <map>
#include <string>
#include <vector>

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

enum ParserState { MALFORMED = -2, INCOMPLETE = -1 };
HttpRequest makeFakeReq() {
	HttpRequest req;
	req.method = "GET";
	req.path = "/index.html";
	req.version = "HTTP/1.1";
	req.headers["Host"] = "localhost:8080";
	req.headers["Connection"] = "keep-alive";
	req.headers["Content-Length"] = "0";
	return req;
}

class HttpParser {
   public:
	int tryParse(char *buff, size_t size, HttpRequest &req) {
		(void)buff;
		(void)size;
		req = makeFakeReq();
		return INCOMPLETE;
	}
};
// TODO: make it inside client.hpp
void queueResponse(const HttpResponse &res, std::vector<u_int8_t> &_wrbuf) {
	std::ostringstream head;

	head << "HTTP/1.0 " << res.status_code << " " << res.status_msg << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it =
			 res.headers.begin();
		 it != res.headers.end(); ++it)
		head << it->first << ": " << it->second << "\r\n";
	head << "Content-Length: " << res.body.size() << "\r\n";
	head << "\r\n";

	std::string headStr = head.str();
	_wrbuf.insert(_wrbuf.end(), headStr.begin(), headStr.end());
	_wrbuf.insert(_wrbuf.end(), res.body.begin(), res.body.end());
}

HttpResponse makeFakeRes() {
	HttpResponse res;
	res.status_code = 200;
	res.status_msg = "OK";
	res.headers["Content-Type"] = "text/plain";

	std::string body = "Hello from webserve\n";
	res.body.insert(res.body.end(), body.begin(), body.end());
	return res;
}
/*  hardcode*/

#define BUFF_SIZE 4096
ClientStatus Client::onReadable() {
	(void)_request_complete;
	(void)_connected_at;
	(void)_last_response_at;
	char buff[BUFF_SIZE];
	char response[BUFF_SIZE];
	int n;
	int parseState;
	HttpRequest req;
	HttpParser parser;

	n = read(fd, buff, sizeof(buff));
	if (n == 0 || n == ERROR) return DISCONNECT;
	// _rbuf.insert(_rbuf.end(), buff, buff + n);

	parseState = parser.tryParse(buff, n, req);
	(void)parseState;
	// {
	// if (parse.state == good) return OK
	// else if (parse.state == error) serveError
	// }
	//
	// if (parse.state == complete){
	// if req.filename.ext == cgi cgi()
	// else if (file.found) serve static file()
	// else if (errfile.found) serve
	// else serve string
	// }
	//

	queueResponse(makeFakeRes(), _wrbuf);

	return WANT_WRITE;
}

ClientStatus Client::onWritable() {
	int n;

	if (hasDataToWrite()) {
		n = write(fd, _wrbuf.data(), _wrbuf.size());
		if (n <= 0) return DISCONNECT;

		_wrbuf.erase(_wrbuf.begin(), _wrbuf.begin() + n);
		if (hasDataToWrite()) return OK;
		return DONE_WRITE;
	}
	return OK;
}

bool Client::hasDataToWrite() const { return !_wrbuf.empty(); }
