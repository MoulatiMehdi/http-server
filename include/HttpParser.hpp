#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "Error.hpp"
#include "HttpRequest.hpp"
#include "State.hpp"
#include <cstddef>
#include <ostream>
#include <string>

class HttpParser
{
  public:
    using Handler = void (HttpParser::*)(u_char);

    static Handler handlers[30];

    // size_t header_line(HttpRequest &request, const char *str, size_t len);
    // size_t request_line(HttpRequest &request, const char *str, size_t len);
    // size_t body(HttpRequest &request, char *str, size_t len);
    //
    void parse(HttpRequest &request, const char *c_str, size_t len);

    void  clear();
    bool  good() const;
    Error error() const;
    State state() const;

    HttpParser();
    HttpParser(const HttpParser &other);
    HttpParser operator=(const HttpParser &other);
    ~HttpParser();

  private:
    State m_state;
    Error m_error;

    std::string    m_target;
    std::string    m_method;
    unsigned short m_major;
    unsigned short m_minor;

    std::string m_header_name;
    std::string m_header_value;

    std::string m_body;

    bool   m_invalid_header;
    size_t m_size;

    void   req_start(u_char ch);
    void   req_method(u_char ch);
    void   req_spaces_before_uri(u_char ch);
    void   req_after_slash_in_uri(u_char ch);
    void   req_check_uri(u_char ch);
    void   req_uri(u_char ch);
    void   req_http_09(u_char ch);
    void   req_http_H(u_char ch);
    void   req_http_HT(u_char ch);
    void   req_http_HTT(u_char ch);
    void   req_http_HTTP(u_char ch);
    void   req_first_major_digit(u_char ch);
    void   req_major_digit(u_char ch);
    void   req_first_minor_digit(u_char ch);
    void   req_minor_digit(u_char ch);
    void   req_spaces_after_digit(u_char ch);
    void   req_almost_done(u_char ch);

    void hdr_start(u_char ch);
    void hdr_name(u_char ch);
    void hdr_space_before_value(u_char ch);
    void hdr_value(u_char ch);
    void hdr_almost_done(u_char ch);
    void hdr_header_almost_done(u_char ch);
};

#endif
