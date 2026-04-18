
#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <string>
void print_response(const HttpResponse &response);
void print_request(const HttpRequest &request);
void print_string(const std::string &str);
void print_string_nl(const std::string &str);
void print_ptr_nl(const char *value, size_t len);
#endif
