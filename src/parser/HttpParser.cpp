#include "HttpParser.hpp"
#include "HttpParserBody.hpp"
#include "HttpParserRequestLine.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "State.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>

HttpParser::Handler HttpParser::handlers[];

HttpParser::HttpParser()
    : HttpParserRequestLine(),
      HttpParserState(),
      HttpParserBody()
{
    handlers[state::s_req_start]  = &HttpParser::req_start;
    handlers[state::s_req_method] = &HttpParser::req_method;
    handlers[state::s_req_spaces_before_uri] =
        &HttpParser::req_spaces_before_uri;
    handlers[state::s_req_uri_after_slash] = &HttpParser::req_uri_after_slash;
    handlers[state::s_req_check_uri]       = &HttpParser::req_check_uri;
    handlers[state::s_req_uri]             = &HttpParser::req_uri;
    handlers[state::s_req_http_09]         = &HttpParser::req_http_09;
    handlers[state::s_req_http_H]          = &HttpParser::req_http_H;
    handlers[state::s_req_http_HT]         = &HttpParser::req_http_HT;
    handlers[state::s_req_http_HTT]        = &HttpParser::req_http_HTT;
    handlers[state::s_req_http_HTTP]       = &HttpParser::req_http_HTTP;
    handlers[state::s_req_first_major_digit] =
        &HttpParser::req_first_major_digit;
    handlers[state::s_req_major_digit] = &HttpParser::req_major_digit;
    handlers[state::s_req_first_minor_digit] =
        &HttpParser::req_first_minor_digit;
    handlers[state::s_req_minor_digit] = &HttpParser::req_minor_digit;
    handlers[state::s_req_spaces_after_digit] =
        &HttpParser::req_spaces_after_digit;
    handlers[state::s_req_almost_done] = &HttpParser::req_almost_done;
    handlers[state::s_hdr_start]       = &HttpParser::hdr_start;
    handlers[state::s_hdr_name]        = &HttpParser::hdr_name;
    handlers[state::s_hdr_space_before_value] =
        &HttpParser::hdr_space_before_value;
    handlers[state::s_hdr_value]       = &HttpParser::hdr_value;
    handlers[state::s_hdr_almost_done] = &HttpParser::hdr_almost_done;
    handlers[state::s_hdr_header_almost_done] =
        &HttpParser::hdr_header_almost_done;
}

void HttpParser::clear()
{
    m_body_size      = 0;
    m_chunk_max_size = 0;
    m_chunk_state    = sw_chunk_start;
    m_chunk_value.clear();

    m_header_name.clear();
    m_header_value.clear();

    m_method.clear();
    m_target.clear();
    m_minor = 0;
    m_major = 0;
    m_state = state::s_req_start;

    m_chunked        = false;
    m_discard_body   = true;
    m_invalid_header = false;
    m_complete       = false;
}

void HttpParser::processRequest(HttpRequest &request, Action &action)
{
    switch (action)
    {
        case PA_REQUEST_LINE_DONE:
            processRequestLine(request);
            break;
        case PA_HEADER_LINE_DONE:
            processHeaderLine(request);
            break;
        case PA_HEADER_DONE:
            processHeaders(request);
            break;
        case PA_ERROR:
        case PA_CONTINUE:
        case PA_BODY_DONE:
        case PA_DONE:
            break;
    }
    processError(request);
}

void HttpParser::parse(HttpRequest &request, const char *c_str, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
    {
        if (!good())
            return;
        if (m_state == state::s_body_start)
            break;
        else
        {
            Action action = (this->*handlers[m_state])(c_str[i]);
            processRequest(request, action);
        }
    }
    if (m_state == state::s_body_start)
    {
        parse_body(request, &c_str[i], len - i);
        if (m_complete)
            request.setComplete(true);
    }
}

HttpParser::~HttpParser()
{
}
