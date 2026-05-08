#include "Cgi.hpp"
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
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

Cgi::Cgi(const std::string &script, const HttpRequest &req)
	: _in(-1),
	  _out(-1),
	  _pid(-1),
	  _parsingHeaders(true),
	  _reqBodyFile(NULL),
	  _req(req),
	  _started_at(time(NULL)) {
	if (_req.body().size() != 0)
		_reqBodyFile = new FileServe(_req.body().c_path());

	_resp.body().open_file();

	int in_pipe[2];
	int out_pipe[2];

	if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0)
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

		// --- build env ---
		std::vector<std::string> envVec;

		envVec.push_back("REQUEST_METHOD=" + to_string(_req.method()));
		envVec.push_back("QUERY_STRING=" + _req.uri().query());
		envVec.push_back("PATH_INFO=");
		envVec.push_back("SCRIPT_NAME=" + _req.uri().path());  //
		envVec.push_back("SERVER_NAME=localhost");
		envVec.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envVec.push_back("GATEWAY_INTERFACE=CGI/1.1");

		std::string contentType = _req.getHeader("Content-Type")->second;
		std::string contentLength = _req.getHeader("Content-Length")->second;
		if (!contentType.empty())
			envVec.push_back("CONTENT_TYPE=" + contentType);
		if (!contentLength.empty())
			envVec.push_back("CONTENT_LENGTH=" + contentLength);

		// HTTP_* variables: pass request headers as HTTP_HEADERNAME
		const HttpMessage::Headers &hdrs = _req.headers();
		for (HttpMessage::const_iterator it = hdrs.begin(); it != hdrs.end();
			 ++it) {
			// skip headers already covered above
			std::string name = it->first;
			if (name == "Content-Type" || name == "Content-Length") continue;

			// convert header name: lowercase with hyphens → uppercase with
			// underscores
			std::string envName = "HTTP_";
			for (size_t i = 0; i < name.size(); ++i) {
				if (name[i] == '-') envName += '_';
				else envName += std::toupper(name[i]);
			}
			envVec.push_back(envName + "=" + it->second);
		}

		// build char** from vector
		char **env = new char *[envVec.size() + 1];
		for (size_t i = 0; i < envVec.size(); ++i)
			env[i] = strdup(envVec[i].c_str());
		env[envVec.size()] = NULL;

		char *argv[] = {const_cast<char *>(script.c_str()), NULL};

		execve(script.c_str(), argv, env);

		for (size_t i = 0; i < envVec.size(); ++i)
			free(env[i]);
		delete[] env;
		_exit(EXIT_FAILURE);
	}

	_in = in_pipe[1];
	_out = out_pipe[0];

	close(in_pipe[0]);
	close(out_pipe[1]);

	makeNonBlocking(_in);
	makeNonBlocking(_out);
}

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

CgiStatus Cgi::_fail(status::Status code) {
	_resp.setStatus(code);
	return CGI_ERROR;
}

CgiStatus Cgi::_finalize() {
	close(_out);
	_out = -1;

	if (!_waitChild()) return _fail(status::BAD_GATEWAY);

	_resp.setStatus(status::OK);
	_resp.setContentLength(_resp.body().size());
	return CGI_DONE;
}

bool Cgi::_waitChild() {
	int status;
	pid_t ret = waitpid(_pid, &status, WNOHANG);

	if (ret == 0) {
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
		_pid = -1;
		return false;
	}

	_pid = -1;

	if (ret == -1) return false;
	if (WIFSIGNALED(status)) return false;
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) return false;

	return true;
}

CgiStatus Cgi::_consume(const char *buff, int n) {
	try {
		if (!_resp.complete()) {
			_resp.parse(buff, n);
			if(!_resp.parser().good())
			    throw ;
			if (_resp.complete()) {
				_resp.body().append(buff + _resp.gcount(), n - _resp.gcount());
			}
		} else {
			_resp.body().append(buff, n);
		}
	} catch (...) { return _fail(status::BAD_GATEWAY); }
	return CGI_OK;
}

CgiStatus Cgi::onReadable() {
	if (_out < 0) return CGI_DONE;

	char buff[BUFF_SIZE];
	int n = read(_out, buff, sizeof(buff));

	if (n == -1) return _fail(status::BAD_GATEWAY);
	if (n == 0) return _finalize();

	return _consume(buff, n);
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

time_t Cgi::startedAt() { return _started_at; }

int Cgi::getOut() const { return _out; }
int Cgi::getIn() const { return _in; }
HttpResponse Cgi::getResponse() {
	_resp.setContentLength(_resp.body().size());
	return _resp;
}
