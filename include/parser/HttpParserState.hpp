#ifndef HTTP_PARSER_STATE_HPP
#define HTTP_PARSER_STATE_HPP

#include "Buffer.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include <ostream>

class HttpParserState
{
  protected:
    enum Phase
    {
        P_REQUEST_LINE = 0,
        P_HEADERS,
        P_BODY,
    };

    typedef unsigned int (HttpParserState::*Handler)(u_char);

    //  request
    unsigned int req_start(u_char ch);
    unsigned int req_method(u_char ch);
    unsigned int req_spaces_before_uri(u_char ch);
    unsigned int req_uri_after_slash(u_char ch);
    unsigned int req_check_uri(u_char ch);
    unsigned int req_uri(u_char ch);
    unsigned int req_http_09(u_char ch);
    unsigned int req_http_H(u_char ch);
    unsigned int req_http_HT(u_char ch);
    unsigned int req_http_HTT(u_char ch);
    unsigned int req_http_HTTP(u_char ch);
    unsigned int req_first_major_digit(u_char ch);
    unsigned int req_major_digit(u_char ch);
    unsigned int req_first_minor_digit(u_char ch);
    unsigned int req_minor_digit(u_char ch);
    unsigned int req_spaces_after_digit(u_char ch);
    unsigned int req_almost_done(u_char ch);

    void process_request_line(unsigned int result);
    void parse_request_line(Buffer &buff);

    // headers
    unsigned int hdr_start(u_char ch);
    unsigned int hdr_name(u_char ch);
    unsigned int hdr_space_before_value(u_char ch);
    unsigned int hdr_value(u_char ch);
    unsigned int hdr_almost_done(u_char ch);
    unsigned int hdr_header_almost_done(u_char ch);

    void parse_headers(Buffer &buff);
    void process_header_line(HttpRequest &request);
    void process_headers(HttpRequest &request);

    void process_content_length(HttpRequest &request);
    void process_transfer_encoding(HttpRequest &request);

    // body
    void parse_body(Buffer &buffer);
    void parse_body_by_chunk(Buffer &buffer);
    void parse_body_by_length(Buffer &buffer);

  protected:
    HttpParserState(HttpRequest &request);
    ~HttpParserState();

    union
    {
        ssize_t m_chunk_max_size;
        ssize_t m_major;
    };

    union
    {
        ssize_t m_chunk_size;
        ssize_t m_minor;
    };

    unsigned int m_state;
    bool         m_chunked;
    Phase        m_phase;
    std::string  m_buff;
    ParserError  m_error;
    HttpRequest &request;

    void processError(HttpRequest &request);
    void setError(ParserError err);
    void clear();

  public:
    bool good() const;
    friend std::ostream &
    operator<<(std::ostream &os, const HttpParserState &hps);
};

std::ostream &operator<<(std::ostream &os, const HttpParserState &hps);
#endif
