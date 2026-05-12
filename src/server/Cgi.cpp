#include "Cgi.hpp"
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <stdexcept>
#include <string>
#include "FileServe.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "Method.hpp"
#include "Router.hpp"
#include "Status.hpp"
#include "helper.hpp"

#include <sstream>
#include <string>

std::string build_command(char *const argv[], char *const envp[]) {
	std::ostringstream oss;

	// env variables
	for (size_t i = 0; envp && envp[i]; ++i) {
		std::string env = envp[i];

		size_t eq = env.find('=');

		if (eq != std::string::npos) {
			oss << env.substr(0, eq + 1) << '\'' << env.substr(eq + 1) << '\'';
		} else {
			oss << '\'' << env << '\'';
		}

		oss << ' ';
	}

	// argv
	for (size_t i = 0; argv && argv[i]; ++i) {
		oss << '\'' << argv[i] << '\'';

		if (argv[i + 1]) oss << ' ';
	}

	return oss.str();
}

Cgi::Cgi(const std::string &cmd, const std::string &script,
		 const HttpRequest &req)
	: _in(-1),
	  _out(-1),
	  _pid(-1),
	  _parsingHeaders(true),
	  _reqBodyFile(NULL),
	  _req(req),
	  // _started_at(std::time(NULL)),
	  _last_activity(std::time(NULL)) {
	Logger::info(std::string("Initializing Cgi"));

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
		std::string dir = Router::getParentDirectory(script);
		std::string fileName = script.substr(dir.size() + 1);
		chdir(dir.c_str());
		Logger::info("CWD: " + dir);
		Logger::info("File name: " + fileName);
		dup2(in_pipe[0], STDIN_FILENO);
		dup2(out_pipe[1], STDOUT_FILENO);

		close(in_pipe[0]);
		close(in_pipe[1]);
		close(out_pipe[0]);
		close(out_pipe[1]);

		// --- build env ---
		std::vector<std::string> envVec;
		std::string full_cmd;

		envVec.push_back("REQUEST_METHOD=" + to_string(_req.method()));
		envVec.push_back("QUERY_STRING=" + _req.uri().query());
		envVec.push_back("PATH_INFO=" + _req.parser().route.pathInfo);
		// envVec.push_back("SCRIPT_NAME="+_req.parser().route.scriptName);  //
		envVec.push_back("SERVER_NAME=localhost");
		envVec.push_back("SERVER_PROTOCOL=HTTP/1.1");
		envVec.push_back("GATEWAY_INTERFACE=CGI/1.1");

		std::string contentType = _req.getHeader("Content-Type")->second;
		std::string contentLength = _req.getHeader("Content-Length")->second;
		if (!contentType.empty())
			envVec.push_back("CONTENT_TYPE=" + contentType);
		if (!contentLength.empty())
			envVec.push_back("CONTENT_LENGTH=" + contentLength);

		const HttpMessage::Headers &hdrs = _req.headers();
		for (HttpMessage::const_iterator it = hdrs.begin(); it != hdrs.end();
			 ++it) {
			std::string name = it->first;
			if (name == "Content-Type" || name == "Content-Length") continue;

			std::string envName = "HTTP_";
			for (size_t i = 0; i < name.size(); ++i) {
				if (name[i] == '-') envName += '_';
				else envName += std::toupper(name[i]);
			}
			envVec.push_back(envName + "=" + it->second);
		}

		char **env = new char *[envVec.size() + 1];
		for (size_t i = 0; i < envVec.size(); ++i)
			env[i] = strdup(envVec[i].c_str());
		env[envVec.size()] = NULL;

		char *argv[] = {const_cast<char *>(cmd.c_str()),
						const_cast<char *>(fileName.c_str()), NULL};

		Logger::info(build_command(argv, env));
		execve(cmd.c_str(), argv, env);

		for (size_t i = 0; i < envVec.size(); ++i)
			free(env[i]);
		delete[] env;
		std::exit(EXIT_FAILURE);
	}

	_in = in_pipe[1];
	_out = out_pipe[0];

	close(in_pipe[0]);
	close(out_pipe[1]);

	makeNonBlocking(_in);
	makeNonBlocking(_out);
	Logger::info(std::string("Cgi initialized"));
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
	Logger::info(std::string("Cgi destroyed"));
}

CgiStatus Cgi::_fail(status::Status code) {
	Logger::info(std::string("Cgi failed"));
	_resp.setStatus(code);
	return CGI_ERROR;
}

CgiStatus Cgi::_finalize() {
	close(_out);
	_out = -1;

	if (!_waitChild()) {
		Logger::error(std::string("\033[31mprocess failed\033[0m: ") +
					  strerror(errno));
		return _fail(status::BAD_GATEWAY);
	}

	_resp.setStatus(status::OK);
	_resp.setContentLength(_resp.body().size());
	return CGI_DONE;
}

bool Cgi::_waitChild() {
	int status;
	pid_t ret = waitpid(_pid, &status, WNOHANG);

	if (ret == 0) {
		kill(_pid, SIGKILL);
		ret = waitpid(_pid, NULL, 0);
		Logger::error("\033[32m return " + toString(ret));
		_pid = -1;
		return true;
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
			if (!_resp.parser().good())
				throw std::runtime_error("Parser Error");
			if (_resp.complete()) {
				_resp.body().append(buff + _resp.gcount(), n - _resp.gcount());
			}
		} else {
			_resp.body().append(buff, n);
		}
	} catch (std::exception &e) {
		Logger::error(e.what());
		return _fail(status::BAD_GATEWAY);
	}
	return CGI_OK;
}

CgiStatus Cgi::onReadable() {
	if (_out < 0) return CGI_DONE;

	char buff[BUFF_SIZE];
	int n = read(_out, buff, sizeof(buff));

	if (n == -1) {
		Logger::error("Cgi::onReadable: " + std::string(strerror(errno)));
		return _fail(status::BAD_GATEWAY);
	}
	if (n == 0) return _finalize();

	_last_activity = std::time(NULL);
	return _consume(buff, n);
}

CgiStatus Cgi::onWritable() {
	if (_in < 0) return CGI_DONE;

	if (!_reqBodyFile) {
		Logger::info(std::string("Cgi::onWritable: ") + "no request body");
		close(_in);
		_in = -1;
		return CGI_DONE;
	}

	int n = _reqBodyFile->sendChunk(_in);
	if (n == ERROR) {
		Logger::error(std::string("Cgi::onWritable: ") +
					  std::string(strerror(errno)));
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

	_last_activity = std::time(NULL);
	return CGI_OK;
}

// time_t Cgi::startedAt() { return _started_at; }
time_t Cgi::lastActivity() { return _last_activity; }

int Cgi::getOut() const { return _out; }
int Cgi::getIn() const { return _in; }
HttpResponse Cgi::getResponse() { return _resp; }
