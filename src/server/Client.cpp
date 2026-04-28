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
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "Router.hpp"
#include "Status.hpp"
#include "helper.hpp"

Client::Client(const ServerConfig &servConf, int fd)
	: _fd(fd),
	  _servConf(servConf),
	  _file(NULL), /* , _connected_at(time(NULL)) */
	  _cgi(NULL) {
	(void)servConf;
}

Client::~Client() {
	if (_fd >= 0) close(_fd);
}

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

// void Client::queueResponse(const HttpResponse &resp, const std::string &body) {
// 	std::string s = resp.to_string();
// 	_wrbuf.insert(_wrbuf.end(), s.begin(), s.end());
// 	_wrbuf.insert(_wrbuf.end(), body.begin(), body.end());
// }

void Client::queueResponse(const std::string &raw) {
	_wrbuf.insert(_wrbuf.end(), raw.begin(), raw.end());
}

Cgi *Client::getCgi() const {
	if (_cgi) return _cgi;
	return NULL;
}

int Client::getFd() const { return _fd; }
ClientStatus Client::onCgiDone() {
	// void status = queueResponse(cgi->getResponse()); // does the
	delete _cgi;
	_cgi = NULL;
	return WANT_WRITE;
}

ClientStatus Client::initCgi(const std::string &path) {
	try {
		_cgi = new Cgi(path, _req);
	} catch (...) {
		return serveErr(status::INTERNAL_SERVER_ERROR), WANT_WRITE;
	}  // log Err
	return INIT_CGI;  // EventLoop takes over from here
}

void Client::serveErr(status::Status code) {
	std::string errPath = _servConf.errorPage(code);

	if (!errPath.empty()) {
		try {
			_file = new FileServe(errPath);
			HttpResponse resp(code);
			resp.setHeader("Content-Type", "text/html");
			resp.setHeader("Content-Length", to_stringg(_file->size()));
			resp.setHeader("Connection", "close");
			return queueResponse(resp.to_string());
		} catch (...) {}  // fall through to built-in
	}

	HttpResponse resp(code);
	std::string raw = resp.serve_page();

	return queueResponse(raw);
}

void Client::serveDir(const std::string &path) {
	HttpResponse resp;
	std::string raw = resp.serve_directory(_servConf.root, path);

	if (!resp.good()) return serveErr(resp.status());
	return queueResponse(raw);
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

ClientStatus Client::handleRoute(const RouteResult &route) {
	// Router::printRouteResult(route);
	switch (route.action) {
		case ROUTE_STATIC_FILE:
			return serveFile(route.path, route.statusCode, route.type), WANT_WRITE;
		case ROUTE_DIRECTORY_LISTING:
			return serveDir(route.path), WANT_WRITE;
		case ROUTE_ERROR:
			return serveErr(route.statusCode), WANT_WRITE;
		case ROUTE_CGI:
			return initCgi(route.path), INIT_CGI;
		case ROUTE_UPLOAD:
			return queueResponse(HttpResponse(status::CREATED).to_string()), WANT_WRITE;
	}
}

ClientStatus Client::onReadable() {
	char buff[BUFF_SIZE];
	int n = read(_fd, buff, sizeof(buff));
	if (n <= 0) return DISCONNECT;

	_req.parse(buff, n);
	if (!_req.good()) return serveErr(_req.status()), WANT_WRITE;
	if (!_req.complete()) return OK;

	return handleRoute(Router::resolve(_servConf, _req));
	// TODO: remove return values, they all return same
}

/* TODO: onCgiDone()
 * - Call queueResponse(_cgi->getResponse()) instead of doing nothing
 * - The CGI response must be built and queued before deleting _cgi
 */

/* TODO: serveErr()
 * - Remove the dead inline HTML body — it's never sent (file takes over)
 * - Replace with _router.buildError(code, _servConf) once Router is wired
 * - The dual path (file vs memory body) is contradictory, pick one
 */

/* TODO: queueResponse()
 * - Uncomment and resolve the initFileServe path — currently commented out
 * - The if (_file) check below is stale logic
 * - HTTP/1.0 should be HTTP/1.1
 */

/* TODO: onReadable()
 * - ROUTE_DIRECTORY_LISTING is stubbed to serveFile("hello.pdf") — implement
 * serveDir()
 * - Unreachable return WANT_WRITE at the bottom after the switch — dead code
 */

/* TODO: Client destructor
 * - No cleanup for _cgi or _file on destruction
 * - Both can leak if client disconnects mid-flight
 */

/* TODO: initCgi()
 * - On failure returns DISCONNECT — should return serveErr(500)
 * - The connection is still valid, don’t drop it
 */

/* TODO: Keep-alive
 * - Everything sends "Connection: close"
 * - Fine for now, but must be revisited when keep-alive is implemented
 */
/* ========================= TODO: TCP / CONNECTION SIDE
 * ========================= */

/*
TODO: Fix epoll_wait timeout
- Currently uses timeout = -1 (wait forever), which violates requirement:
"A request should never hang indefinitely"
- Replace with a finite timeout (e.g. 5000 ms)
- On timeout, run a maintenance pass over all clients
*/

/*
TODO: Implement client timeout tracking
- Add timestamp per client (request start time)
- On each epoll_wait timeout:
- Iterate over _cliTable
- Compute elapsed time
- Disconnect clients exceeding timeout threshold
- Revive _connected_at or equivalent field
*/

/*
TODO: Fix Client destructor resource leaks
- Currently leaks:
- _cgi
- _file
- Ensure proper deletion/cleanup of both
- If CGI is active:
- Close pipes
- Remove pipe fds from epoll
- Clean _pipe_to_client mappings
*/

/*
TODO: Fix disconnectClient() incomplete cleanup
- Currently:
- Removes client fd from epoll
- Deletes client
- Missing:
- Remove CGI pipe fds from epoll
- Erase entries from _pipe_to_client
- Prevent dangling pipe fds and stale mappings
*/

/* ============================== TODO: CGI SIDE ==============================
 */

/*
TODO: Fix pipe() critical bug
- Current bug:
pipe(out_pipe) called twice, in_pipe never initialized
- Fix:
if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0)
- Without this:
- Child dup2 uses garbage fd
- CGI completely broken
*/

/*
TODO: Implement CGI timeout handling
- Add _started_at timestamp in Cgi
- During epoll timeout maintenance:
- Check execution duration
- If exceeded:
- call cgikill()
- return CGI_ERROR to client
- Prevent hanging CGI processes
*/

/*
TODO: Implement CGI response handling in onCgiDone()
- Current behavior:
- Deletes _cgi
- Discards output
- Required:
- Parse _cgi->_output:
- Status line
- Headers
- Body
- Build proper HTTP response
- Call queueResponse()
*/

/*
TODO: Fix CGI environment variables (RFC 3875 compliance)
- Current behavior:
- Passing raw HTTP headers as env vars
- Incorrect: CGI expects specific variables
- Must include:
- REQUEST_METHOD
- CONTENT_LENGTH
- CONTENT_TYPE
- QUERY_STRING
- PATH_INFO
- SCRIPT_FILENAME
- etc.
- Properly map HTTP request → CGI env
*/

/*
TODO: Handle env memory on execve failure
- env[] is built with strdup()
- On execve success: OK (process replaced)
- On failure:
- Must free allocated env entries before _exit
- Minor leak but should be fixed
*/

/* ============================== PRIORITY ============================== */

/*
CRITICAL:
- Fix pipe() bug

HIGH:
- epoll_wait timeout
- client timeout tracking
- disconnectClient cleanup
- destructor leaks

MEDIUM:
- CGI timeout
- CGI response parsing
- CGI env correctness
- env cleanup on execve failure
*/

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
