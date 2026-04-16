#ifndef CGI_HPP
#define CGI_HPP
#include <sys/types.h>
#include <string>
#include <vector>
#include "FileServe.hpp"
#include "HttpRequest.hpp"

enum CgiStatus {
	CGI_OK,
	CGI_WANT_WRITE, 
	CGI_DONE_WRITE, 
	CGI_DISCONNECT
};

class Cgi {
   private:
	int _in;
	int _out;
	pid_t _pid;
	int _write_offset;
	FileServe *_reqBodyFile;
	const HttpRequest &_req;

   public:
	Cgi(const std::string &script, const HttpRequest &req);
	~Cgi();
	CgiStatus onWritable();
	CgiStatus onReadable();
	bool done() const;
	std::vector<u_int8_t> &output();
	void cgikill();
	void getPipe(int *) const;
};

#endif	// !CGI_HPP
