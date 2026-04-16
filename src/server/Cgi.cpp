#include "Cgi.hpp"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "FileServe.hpp"
#include "helper.hpp"

Cgi::Cgi(const std::string &script, const HttpRequest &req)
	: _in(-1), _out(-1), _pid(-1), _write_offset(0), _req(req) {
	_reqBodyFile = new FileServe(_req.body().c_path());

	int in_pipe[2];
	int out_pipe[2];

	if (_reqBodyFile->done()) {
		delete _reqBodyFile;
		throw std::runtime_error("FileServe failed");
	}

	if (pipe(in_pipe) < 0 || pipe(out_pipe) < 0)
		throw std::runtime_error("pipe failed");
	// TODO: catch later, also see if you can throw in
	// other constructors

	_pid = fork();
	if (_pid < 0) throw std::runtime_error("fork failed");

	if (_pid == 0) {
		// stdin
		dup2(in_pipe[STDIN_FILENO], STDIN_FILENO);
		// stdout
		dup2(out_pipe[STDOUT_FILENO], STDOUT_FILENO);

		close(in_pipe[STDOUT_FILENO]);
		close(out_pipe[STDIN_FILENO]);
		close(in_pipe[STDIN_FILENO]);
		close(out_pipe[STDOUT_FILENO]);

		std::vector<char *> env;

		env.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
		env.push_back(strdup("REQUEST_METHOD=GET"));  // TODO: from req
		env.push_back(strdup("SCRIPT_NAME=/cgi-bin/script"));
		env.push_back(NULL);

		char *argv[2];
		argv[0] = const_cast<char *>(script.c_str());
		argv[1] = NULL;

		execve(script.c_str(), argv, env.data());
		_exit(EXIT_FAILURE);
	}

	_in = in_pipe[STDOUT_FILENO];
	_out = out_pipe[STDIN_FILENO];

	close(in_pipe[STDIN_FILENO]);
	close(out_pipe[STDOUT_FILENO]);

	make_non_blocking(_in);	 // TODO: change name to camelCase
	make_non_blocking(_out);
}

void Cgi::getPipe(int *arr) const {
	arr[0] = _in;
	arr[1] = _out;
}

CgiStatus Cgi::onWritable() {
	if (_in < 0) return CGI_DONE_WRITE;
	if (_reqBodyFile) {
		if (_reqBodyFile->sendChunk(_in) == ERROR)
			throw std::runtime_error("sendChunk failed");
	}
	if (_reqBodyFile->done()) {
		delete _reqBodyFile;
		_reqBodyFile = NULL;
		return CGI_DONE_WRITE;	// remove only when done write
	}
	return CGI_OK;
	//
	// while (_write_offset < (int)_req.body().size()) {
	// 	ssize_t n = write(_in, &_req.body() + _write_offset,
	// 					  dummy.size() - _write_offset);
	//
	// 	if (n > 0) {
	// 		_write_offset += n;
	// 	} else {
	// 		if (errno == EAGAIN || errno == EWOULDBLOCK) return;
	// 		// real error → close
	// 		close(_in);
	// 		_in = -1;
	// 		return;
	// 	}
	// }

	// finished writing → close stdin to signal EOF to CGI
	// close(_in);
	// _in = -1;

	// if (_tmp_len == 0) {
	// 	int n = read(_fd, _tmp, sizeof(_tmp));
	// 	if (n == 0) {
	// 		close(_fd);
	// 		_fd = -1;
	// 		return 0;
	// 	}
	// 	if (n == ERROR) {
	// 		close(_fd);
	// 		_fd = -1;
	// 		return ERROR;
	// 	}
	// 	_tmp_len = n;
	// 	_tmp_offset = 0;
	// }
	// int n = write(client_fd, _tmp + _tmp_offset, _tmp_len - _tmp_offset);
	// if (n == ERROR) return ERROR;
	// _tmp_offset += n;
	//
	// if (_tmp_offset == _tmp_len) {
	// 	_tmp_len = 0;
	// 	_tmp_offset = 0;
	// }
	//
	// return n;
}

CgiStatus Cgi::onReadable() {
	// must buffer reading until finding headers's end
	// then stream everything into client
	if (_out < 0) return CGI_WANT_WRITE;

	char buff[BUFF_SIZE];
	int n = read(_out, buff, sizeof(buff));
	if (n == 0 || n == ERROR) {
		close(_out);
		_out = -1;
		return CGI_DISCONNECT;
	}
	std::cout << "\ncgi output: ";
	std::cout.write(buff, n);
	std::cout << "\n\n";
	return CGI_WANT_WRITE;
	// parse(buff, n, resp); // must fill a response
	// if (!_req.good()) {
	// 	this->kill();
	// return
	// }

	// while (true) {
	// 	ssize_t n = read(_out, buffer, sizeof(buffer));
	//
	// 	if (n > 0) {
	// 		_output.insert(_output.end(), buffer, buffer + n);
	// 	} else if (n == 0) {
	// 		// EOF → child closed stdout
	// 		close(_out);
	// 		_out = -1;
	// 		return;
	// 	} else {
	// 		if (errno == EAGAIN || errno == EWOULDBLOCK) return;
	// 		// error
	// 		close(_out);
	// 		_out = -1;
	// 		return;
	// 	}
	// }
}

// bool Cgi::done() const {
// 	if (_out != -1) return false;
//
// 	int status;
// 	pid_t result = waitpid(_pid, &status, WNOHANG);
//
// 	if (result == 0) return false;
//
// 	return true;
// }

// std::vector<u_int8_t> &Cgi::output() { return _output; }

void Cgi::cgikill() {
	if (_pid > 0) ::kill(_pid, SIGKILL);

	if (_in != -1) close(_in);
	if (_out != -1) close(_out);

	_in = -1;
	_out = -1;
}

Cgi::~Cgi() {
	// 1. Close fds (safe even if already closed)
	if (_in != -1) {
		close(_in);
		_in = -1;
	}

	if (_out != -1) {
		close(_out);
		_out = -1;
	}

	// 2. Handle child process
	if (_pid > 0) {
		int status;

		// Check if already exited
		pid_t ret = waitpid(_pid, &status, WNOHANG);

		if (ret == 0) {
			// Still running → kill it
			kill(_pid, SIGKILL);

			// Now we MUST reap it (blocking is acceptable here)
			waitpid(_pid, &status, 0);
		}
		// else:
		// ret > 0 → already reaped
		// ret == -1 → already handled or error (ignore safely)

		_pid = -1;
	}
}
