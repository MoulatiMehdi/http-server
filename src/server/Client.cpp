#include "Client.hpp"
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>

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

// struct HttpRequest {
// 	std::string method;
// 	std::string path;
// 	std::string version;
// 	std::map<std::string, std::string> headers;
// 	std::vector<u_int8_t> body;
// };
// struct HttpResponse {
// 	int status_code;
// 	std::string status_msg;
// 	std::map<std::string, std::string> headers;
// 	std::vector<u_int8_t> body;
//
// 	HttpResponse() : status_code(200), status_msg("OK") {}
// };
// struct RouterResult {
// 	bool cgi;
// 	std::string cgi_path;	// if cgi == true
// 	HttpResponse response;	// if cgi == false
// };

// void hardcode(RouterResult &rres, HttpResponse &res, HttpRequest &req,
// std::string buff) { 	req.method = "GET"; 	req.path = "/index.html";
// req.version = "HTTP/1.1"; 	req.headers["Host"] = "localhost:8080";
// 	req.headers["Connection"] = "keep-alive";
//
// 	std::string body = "Hello from webserve\n";
// 	body += "You have sent: " + buff + "\n";
// 	res.status_code = 200;
// 	res.status_msg = "OK";
// 	res.headers["Content-Type"] = "text/html";
// 	res.body.insert(res.body.end(), body.begin(), body.end());
//
// 	rres.cgi = false;
// 	rres.response = res;
// }
// /*  hardcode*/

#define BUFF_SIZE 4096

#include "HttpParser.hpp"
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
	if (resp.isFile) readFile(resp.path.c_str(), _wrbuf);
	else _wrbuf.insert(_wrbuf.end(), resp.body.begin(), resp.body.end());

	return WANT_WRITE;
}

#include <sys/stat.h>
#include "helper.hpp"
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
		to_stringg(code) + " " + msg +
		"</h1>\n"
		"</body>\n"
		"</html>\n";

	// Fill body as bytes
	resp.body.assign(body.begin(), body.end());

	// Headers
	resp.isFile = true;
	resp.path = "./err.html";
	struct stat st;
	stat(resp.path.c_str(), &st);
	resp.headers["Content-Type"] = "text/html";
	resp.headers["Content-Length"] = to_stringg(st.st_size);
	resp.headers["Connection"] = "close";
	return queueResponse(resp);
}

ClientStatus Client::onReadable() {
	int n;
	char buff[BUFF_SIZE];
	n = read(_fd, buff, sizeof(buff));
	if (n == 0 || n == ERROR) return DISCONNECT;
	std::cout.write(buff, n);

	return serveErr(400);
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
