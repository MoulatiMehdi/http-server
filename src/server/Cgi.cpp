#include "Cgi.hpp"
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "helper.hpp"

Cgi::Cgi(const std::string &script, const HttpRequest &req)
	: _in(-1), _out(-1), _pid(-1), _req(req) {
	_reqBodyFile = NULL;
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

		char *env[] = {strdup("GATEWAY_INTERFACE=CGI/1.1"),
					   strdup("REQUEST_METHOD=GET"),
					   strdup("SCRIPT_NAME=/cgi-bin/script"), NULL};

		char *argv[] = {const_cast<char *>(script.c_str()), NULL};

		execve(script.c_str(), argv, env);
		_exit(EXIT_FAILURE);
	}

	// parent keeps: write end of in_pipe, read end of out_pipe
	_in = in_pipe[1];
	_out = out_pipe[0];

	close(in_pipe[0]);
	close(out_pipe[1]);

	make_non_blocking(_in);
	make_non_blocking(_out);
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

	while (true) {
		char buff[BUFF_SIZE];
		int n = read(_out, buff, sizeof(buff));

		if (n == -1) {
			close(_out);
			_out = -1;
			return CGI_ERROR;
		}

		if (n == 0) {
			close(_out);
			_out = -1;
			waitpid(_pid, NULL, WNOHANG);
			_pid = -1;
			return CGI_DONE;
		}

		_output.insert(_output.end(), buff, buff + n);
	}
}

void Cgi::cgikill() {
	if (_pid > 0) ::kill(_pid, SIGKILL);

	if (_in != -1) close(_in);
	if (_out != -1) close(_out);

	_in = -1;
	_out = -1;
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
