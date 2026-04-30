#ifndef HTTP_PARSER_STATE_HPP
#define HTTP_PARSER_STATE_HPP

#include "Buffer.hpp"
#include "HttpMessage.hpp"
#include "ParserError.hpp"
#include <ostream>

class HttpParserState
{
  protected:
    enum HeaderResult
    {
        RES_ERROR,
        RES_CONTINUE,
        RES_HEADER_DONE,
        RES_HEADER_LINE_DONE
    };

    enum Phase
    {
        PHASE_REQUEST_LINE = 0,
        PHASE_HEADERS,
        PHASE_BODY,
    };

    typedef unsigned int (HttpParserState::*Handler)(u_char);

    // headers
    unsigned int hdr_start(u_char ch);
    unsigned int hdr_name(u_char ch);
    unsigned int hdr_space_before_value(u_char ch);
    unsigned int hdr_value(u_char ch);
    unsigned int hdr_almost_done(u_char ch);
    unsigned int hdr_header_almost_done(u_char ch);

    void parse_headers(Buffer &buff);
    void process_header_line(HttpMessage &request);
    void process_headers(HttpMessage &request);

    void process_host();
    void process_content_length(HttpMessage &request);
    void process_transfer_encoding(HttpMessage &request);

  protected:
    static const int MAX_BUFFER = 4096;
    HttpParserState(HttpMessage &request);
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

    int          m_parsed;
    unsigned int m_state;
    bool         m_chunked;
    bool         m_discard_body;
    Phase        m_phase;
    std::string  m_buff;
    ParserError  m_error;
    HttpMessage &request;

    virtual void process_error() = 0;
    void         setError(ParserError err);
    void         clear();

  public:
    bool good() const;
    friend std::ostream &
    operator<<(std::ostream &os, const HttpParserState &hps);
};

std::ostream &operator<<(std::ostream &os, const HttpParserState &hps);
#endif
