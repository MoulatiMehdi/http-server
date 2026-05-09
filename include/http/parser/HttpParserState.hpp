#ifndef HTTP_PARSER_STATE_HPP
#define HTTP_PARSER_STATE_HPP

#include "HttpMessage.hpp"
#include "ParserError.hpp"

class HttpParserState
{
  public:
    enum HeaderResult
    {
        RES_HDR_ERROR,
        RES_HDR_CONTINUE,
        RES_HEADER_DONE,
        RES_HEADER_LINE_DONE
    };

    enum RequestLineResult
    {
        RES_ERROR,
        RES_CONTINUE,
        RES_METHOD_DONE,
        RES_URI_DONE,
        RES_VERSION_DONE,
        RES_REQUEST_LINE_DONE
    };

    enum Phase
    {
        PHASE_REQUEST_LINE = 0,
        PHASE_HEADERS,
        PHASE_BODY,
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
    // headers
    unsigned int hdr_start(u_char ch);
    unsigned int hdr_name(u_char ch);
    unsigned int hdr_space_before_value(u_char ch);
    unsigned int hdr_value(u_char ch);
    unsigned int hdr_almost_done(u_char ch);
    unsigned int hdr_header_almost_done(u_char ch);

    virtual void process_headers()        = 0;
    virtual void process_content_length() = 0;
    void         process_host();
    void         process_transfer_encoding();
    void         process_header_line();

  protected:
    static const int MAX_HEADERS_BUFFER = 4096;
    static const int MAX_REQUEST_BUFFER = 1024;

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

    int          m_parsed;
    unsigned int m_state;
    bool         m_chunked;
    bool         m_discard_body;
    Phase        m_phase;
    std::string  m_buff;
    ParserError  m_error;
    HttpMessage &m_message;

    void setError(ParserError err);
    void clear();

  public:
    HttpParserState(HttpMessage &request);
    virtual ~HttpParserState();
    virtual void process_error() = 0;

    bool good() const;
    // friend std::ostream &
    // operator<<(std::ostream &os, const HttpParserState &hps);
};

// std::ostream &operator<<(std::ostream &os, const HttpParserState &hps);
#endif
