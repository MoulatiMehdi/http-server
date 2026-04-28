#include "Cgi.hpp"
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "FileServe.hpp"
#include "HttpResponse.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include "helper.hpp"

// TODO: add CGI timeout — track start time, kill child if it exceeds config
// limit
//       waitpid(WNOHANG) in EventLoop maintenance tick, kill + CGI_ERROR if
//       exceeded

Cgi::Cgi(const std::string &script, const HttpRequest &req)
	: _in(-1),
	  _out(-1),
	  _pid(-1),
	  _parsingHeaders(true),
	  _reqBodyFile(NULL),
	  _req(req),
	  _started_at(time(NULL)) {
	if (_req.body().size() != 0)
		_reqBodyFile = new FileServe(_req.body().c_path());	 // POST/PUT???

	_resp.body().open_file();

	int in_pipe[2];
	int out_pipe[2];

	if (pipe(out_pipe) < 0 || pipe(in_pipe) < 0)
		throw std::runtime_error("pipe failed");

	_pid = fork();
	if (_pid < 0) throw std::runtime_error("fork failed");

	if (_pid == 0) {
		dup2(in_pipe[0], STDIN_FILENO);
		dup2(out_pipe[1], STDOUT_FILENO);

		close(in_pipe[0]);
		close(in_pipe[1]);
		close(out_pipe[0]);
		close(out_pipe[1]);

		const HttpMessage::Headers &hdrs = _req.headers();
		char **env = new char *[hdrs.size() + 1];

		int i = 0;
		for (HttpMessage::const_iterator it = hdrs.begin(); it != hdrs.end();
			 it++) {
			std::cout << it->first + "=" + it->second << std::endl;	 // DEBUG:
			std::string entry = it->first + "=" + it->second;
			env[i++] = strdup(entry.c_str());
		}
		env[i] = NULL;

		char *argv[] = {const_cast<char *>(script.c_str()), NULL};

		execve(script.c_str(), argv, env);
		_exit(EXIT_FAILURE);
	}

	_in = in_pipe[1];
	_out = out_pipe[0];

	close(in_pipe[0]);
	close(out_pipe[1]);

	makeNonBlocking(_in);
	makeNonBlocking(_out);
}

CgiStatus Cgi::onWritable() {
	if (_in < 0) return CGI_DONE;

	if (!_reqBodyFile) {
		close(_in);
		_in = -1;
		return CGI_DONE;
	}

	int n = _reqBodyFile->sendChunk(_in);
	if (n == ERROR) {
		close(_in);
		_in = -1;
		return CGI_ERROR;
	}

	if (_reqBodyFile->done()) {
		delete _reqBodyFile;
		_reqBodyFile = NULL;
		close(_in);
		_in = -1;
		return CGI_DONE;
	}

	return CGI_OK;
}

CgiStatus Cgi::onReadable() {
	if (_out < 0) return CGI_DONE;

	char buff[BUFF_SIZE];
	int n = read(_out, buff, sizeof(buff));

	if (n == -1) {
		_resp.setStatus(status::BAD_GATEWAY);
		return CGI_ERROR;
	}

	if (n == 0) {
		close(_out);
		_out = -1;
		waitpid(_pid, NULL, WNOHANG);
		_pid = -1;
		return CGI_DONE;
	}

	try {
		if (!_resp.complete()) {
			_resp.parse(buff, n);
			if (_resp.complete()) {
				_resp.body().append(buff + _resp.gcount(), n - _resp.gcount());
				_parsingHeaders = false;
			}
		} else if (!_parsingHeaders) _resp.body().append(buff, n);
	} catch (const std::exception &e) {
		_resp.setStatus(status::BAD_GATEWAY);
		return CGI_ERROR;
	}
	return CGI_OK;
}

time_t Cgi::startedAt() { return _started_at; }
Cgi::~Cgi() {
	if (_in != -1) {
		close(_in);
		_in = -1;
	}

	if (_out != -1) {
		close(_out);
		_out = -1;
	}

	if (_pid > 0) {
		int status;
		pid_t ret = waitpid(_pid, &status, WNOHANG);
		if (ret == 0) {
			kill(_pid, SIGKILL);

			waitpid(_pid, &status, 0);
		}
		_pid = -1;
	}
}

HttpResponse Cgi::getResponse() {
	_resp.setContentLength(_resp.body().size());
	return _resp;
}
