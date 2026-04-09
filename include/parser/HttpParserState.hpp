#ifndef HTTP_PARSER_STATE_HPP
#define HTTP_PARSER_STATE_HPP

#include "Error.hpp"
#include "HttpRequest.hpp"
#include "State.hpp"

class HttpParserState
{
  private:
    HttpParserState(HttpParserState &other);
    HttpParserState &operator=(HttpParserState &other);
    Error            m_error;

  protected:
    enum Action
    {
        PA_DONE = 0,
        PA_CONTINUE,
        PA_ERROR,
        PA_REQUEST_LINE_DONE,
        PA_HEADER_LINE_DONE,
        PA_HEADER_DONE,
        PA_BODY_DONE,
    };

    using u_char = unsigned char;

    HttpParserState();
    ~HttpParserState();

    State m_state;

    bool m_invalid_header;
    bool m_complete;
    bool m_discard_body;
    bool m_chunked;

    void processError(HttpRequest &request);
    void setError(Error err);

  public:
    bool  good() const;
    Error error() const;
    State state() const;
};
#endif
