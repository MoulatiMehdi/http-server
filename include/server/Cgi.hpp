#ifndef CGI_HPP
#define CGI_HPP
#include <sys/types.h>
#include <string>
#include <vector>
#include "FileServe.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

enum CgiStatus { CGI_OK, CGI_DONE, CGI_ERROR };

class Cgi {
   private:
	int _in;
	int _out;
	pid_t _pid;
	int _write_offset;
	bool _parsingHeaders;
	FileServe *_reqBodyFile;
	const HttpRequest &_req;
	HttpResponse _resp;
	FileServe *_file;
	std::vector<u_int8_t> _output; // REMOVE?

   public:
	Cgi(const std::string &script, const HttpRequest &req);
	~Cgi();
	CgiStatus onWritable();
	CgiStatus onReadable();
	bool done() const;
	std::vector<u_int8_t> &output() { return _output; };
	void cgikill();
	int getIn() const { return _in; }
	int getOut() const { return _out; }
	HttpResponse getResponse();
};

#endif	// !CGI_HPP
