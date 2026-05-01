#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>

#include "FileServe.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Status.hpp"

enum CgiStatus { CGI_OK, CGI_DONE, CGI_ERROR };

class Cgi {
   public:
	Cgi(const std::string &script, const HttpRequest &req);
	~Cgi();

   private:
	bool _waitChild();
	void cgikill();

	CgiStatus _consume(const char *buff, int n);
	CgiStatus _finalize();

	CgiStatus _fail(status::Status code);


   public:
	CgiStatus onReadable();
	CgiStatus onWritable();

	time_t startedAt();
	int getIn() const;
	int getOut() const;
	HttpResponse getResponse();

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

	time_t _started_at;
};

#endif
