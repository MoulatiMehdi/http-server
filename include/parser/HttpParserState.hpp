#ifndef HTTP_PARSER_STATE_HPP
#define HTTP_PARSER_STATE_HPP

#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include <ostream>

class HttpParserState
{
  private:
    HttpParserState(HttpParserState &other);
    HttpParserState &operator=(HttpParserState &other);
    ParserError      m_error;

  protected:
    enum Action
    {
        PA_OK = 0,
        PA_CONTINUE,
        PA_DONE,
        PA_ERROR,
    };

    enum Phase
    {
        P_REQUEST_LINE = 0,
        P_HEADERS,
        P_BODY,
    };

    HttpParserState();
    ~HttpParserState();

    unsigned int m_state;
    bool         m_complete;
    bool         m_discard_body;
    bool         m_chunked;
    Phase        m_phase;
    std::string  m_cache;

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
