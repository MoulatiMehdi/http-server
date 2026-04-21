
#include <csignal>
#include <iostream>
#include "Config.hpp"
#include "ConfigParser.hpp"
#include "HttpServer.hpp"
// TODO: add signal(SIGPIPE, SIG_IGN) at server startup in main or HttpServer::init()

// TODO: implement Router — takes HttpRequest + ServerConfig, returns RouterResult
//       RouterResult: { bool isCgi, string cgiPath, HttpResponse response }

// TODO: implement HttpParser — feed(buf, n, req), req.good(), req.complete()
//       must handle chunked requests and Content-Length body accumulation

// TODO: wire _servConf into Client properly for:
//       - error page lookup in serveErr()
//       - max body size enforcement in parser
//       - CGI env var population
// TODO: remove all debug std::cout statements before final submission
//       replace with Logger::debug() calls gated behind a DEBUG flag

// TODO: remove printEpollEvents() debug helper from EventLoop

// TODO: remove static int x hack in Client::onReadable() — it is not thread-safe
//       and will break on the second request from the same client

// TODO: remove commented-out dead code throughout all files:
//       old RouterResult struct, readFile(), unused includes, commented-out flows
//       if it is not being used now, it does not belong in the codebase

// TODO: remove the _cgi_pending field from Client — it is set but never read
//       either give it a real purpose or delete it entirely

// TODO: enforce consistent naming convention across all files:
//       camelCase for methods, _snake_case for members — currently mixed
//       make_non_blocking() should be makeNonBlocking()

// TODO: move #define BUFF_SIZE out of Client.cpp into a shared header (common/defines.hpp)
//       it is used in multiple files and should not be defined in a .cpp

// TODO: add header guards or #pragma once to every .hpp file — audit all includes

// TODO: every class that owns heap resources must have:
//       private copy constructor and copy assignment operator declared but not defined
//       Client, Cgi, FileServe, Socket are all non-copyable and must enforce it

// TODO: audit every constructor — members must be initialized in the initializer list
//       in the same order they are declared in the class, not mixed order

// TODO: replace raw NULL with a named sentinel or at minimum audit every NULL check
//       for consistency — pick one style and stick to it across the codebase

// TODO: FileServe constructor silently returns on open/fstat failure with _fd = -1
//       this makes the caller check done() to detect failure which is not obvious
//       consider throwing or adding a clear isValid() contract in the header comment

// TODO: write a top-of-file block comment for each class explaining:
//       what it owns, what its invariants are, and what it does NOT do
//       example for Cgi: "owns the child process and both pipe fds.
//       does not register fds with epoll — that is EventLoop's responsibility"

// TODO: split EventLoop::loop() dispatch into clearly named private helpers
//       the if/else if chain in the loop body should read like documentation:
//       isListeningSocket(), isClientFd(), isCgiPipe() as named predicates

// TODO: audit all error paths — exitError() should only be called for truly
//       unrecoverable server errors (epoll_create, bind, listen)
//       recoverable per-client errors must not take down the whole server
//       THROW
int main(int ac, char **av) {
	if (ac != 2) {
		std::cout << "Invalide arguments" << std::endl;
		std::cout << "  Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	HttpServer server(av[1]);  // TODO: rename to HttpServer

	signal(SIGPIPE, SIG_IGN);
	// try catch
	server.init();
	server.run();
	return 0;
}
