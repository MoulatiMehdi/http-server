#include "HttpParser.hpp"
#include "Error.hpp"
#include "HttpRequest.hpp"
#include <cstring>
#include <iostream>

HttpParser::Handler HttpParser::handlers[];

HttpParser::HttpParser()
    : m_error(error::ok),
      m_state(state::s_req_start),
      m_major(0),
      m_minor(0),
      m_header_name(),
      m_header_value(),
      m_method(),
      m_target(),
      m_invalid_header(false)

{
    handlers[state::s_req_start]  = &HttpParser::req_start;
    handlers[state::s_req_method] = &HttpParser::req_method;
    handlers[state::s_req_spaces_before_uri] =
        &HttpParser::req_spaces_before_uri;
    handlers[state::s_req_after_slash_in_uri] =
        &HttpParser::req_after_slash_in_uri;
    handlers[state::s_req_check_uri] = &HttpParser::req_check_uri;
    handlers[state::s_req_uri]       = &HttpParser::req_uri;
    handlers[state::s_req_http_09]   = &HttpParser::req_http_09;
    handlers[state::s_req_http_H]    = &HttpParser::req_http_H;
    handlers[state::s_req_http_HT]   = &HttpParser::req_http_HT;
    handlers[state::s_req_http_HTT]  = &HttpParser::req_http_HTT;
    handlers[state::s_req_http_HTTP] = &HttpParser::req_http_HTTP;
    handlers[state::s_req_first_major_digit] =
        &HttpParser::req_first_major_digit;
    handlers[state::s_req_major_digit] = &HttpParser::req_major_digit;
    handlers[state::s_req_first_minor_digit] =
        &HttpParser::req_first_minor_digit;
    handlers[state::s_req_minor_digit] = &HttpParser::req_minor_digit;
    handlers[state::s_req_spaces_after_digit] =
        &HttpParser::req_spaces_after_digit;
    handlers[state::s_req_almost_done] = &HttpParser::req_almost_done;
    handlers[state::s_req_done]        = &HttpParser::hdr_start;
    handlers[state::s_hdr_start]       = &HttpParser::hdr_start;
    handlers[state::s_hdr_name]        = &HttpParser::hdr_name;
    handlers[state::s_hdr_space_before_value] =
        &HttpParser::hdr_space_before_value;
    handlers[state::s_hdr_value]       = &HttpParser::hdr_value;
    handlers[state::s_hdr_almost_done] = &HttpParser::hdr_almost_done;
    handlers[state::s_hdr_header_almost_done] =
        &HttpParser::hdr_header_almost_done;
}

HttpParser::~HttpParser()
{
}

bool HttpParser::good() const
{
    return m_error == error::ok;
}

Error HttpParser::error() const
{
    return m_error;
}

State HttpParser::state() const
{
    return m_state;
}

void HttpParser::parse(HttpRequest &request, const char *c_str, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
    {
        if (m_error != error::ok)
            return;
        switch (m_state)
        {
            case state::s_req_done:
                break;
            case state::s_hdr_line_done:
                break;
            case state::s_hdr_done:
                break;
            case state::s_body_start:
                break;
            default:
                (this->*handlers[m_state])(c_str[i]);
                break;
        }
        if (m_state == state::s_req_done)
        {
            request.set_method(string_to_method(m_method));
            request.set_target(m_target);
            request.set_version(m_major, m_minor);
            if (request.version() == 9)
            {
                m_error = error::unsupported_version;
                return;
            }
            m_state = state::s_hdr_start;
        }
        if (m_state == state::s_hdr_line_done)
        {
            request.setHeader(m_header_name, m_header_value);
            m_state = state::s_hdr_start;
        }
        if (m_state == state::s_hdr_done)
        {
            i++;
            m_state = state::s_body_start;
            break;
        }
    }
    m_size += i;
    if (m_state == state::s_body_start && i < len)
        request.body().append(&c_str[i], len - i);
}
