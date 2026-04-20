#ifndef CGI_HPP
#define CGI_HPP
#include <sys/types.h>
#include <string>
#include <vector>
#include "FileServe.hpp"
#include "HttpRequest.hpp"

enum CgiStatus { CGI_OK, CGI_DONE, CGI_ERROR };

class Cgi {
   private:
	int _in;
	int _out;
	pid_t _pid;
	int _write_offset;
	FileServe *_reqBodyFile;
	const HttpRequest &_req;

   public:
	std::vector<u_int8_t> _output;
	Cgi(const std::string &script, const HttpRequest &req);
	~Cgi();
	CgiStatus onWritable();
	CgiStatus onReadable();
	bool done() const;
	std::vector<u_int8_t> &output();
	void cgikill();
	int getIn() const { return _in; }
	int getOut() const { return _out; }
};

#endif	// !CGI_HPP
