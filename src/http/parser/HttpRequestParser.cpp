#include "HttpRequestParser.hpp"
#include "BodyStorage.hpp"
#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "MimeType.hpp"
#include "ParserError.hpp"
#include "RouteResult.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <strings.h>

HttpRequestParser::HttpRequestParser(HttpRequest &request)
    : HttpParserState(request),
      m_request(request),
      m_boundary(),
      m_index(0),
      m_response()
{
}

void HttpRequestParser::process_content_type()
{
    HttpRequest::const_iterator it = m_request.getHeader("content-type");

    if (it == m_request.headers().end())
        return setError(error::bad_request);

    std::istringstream iss(it->second);
    iss >> m_content_type;
    if (m_content_type[m_content_type.size() - 1] == ';')
        m_content_type.erase(m_content_type.size() - 1);

    if (m_content_type == "multipart/form-data")
    {
        m_boundary = m_message.extract_key("content-type", "boundary");
        if (m_boundary.empty())
            return setError(error::bad_request);
        m_boundary = "--" + m_boundary;
    }
    else
    {
        m_filename = route.path + "/" + BodyStorage::generateName() + ".txt";
        m_request.body().open_file(m_filename, false);
    }
}

void HttpRequestParser::parse(const char *c_str, size_t len)
{
    if (m_request.complete() || !m_request.good())
        return;
    Buffer buffer(c_str, len);
    while (!buffer.empty())
    {
        switch (m_phase)
        {
            case HttpParserState::PHASE_REQUEST_LINE:
                parse_request_line(buffer);
                break;
            case HttpParserState::PHASE_HEADERS:
                parse_headers(buffer);
                break;
            case HttpParserState::PHASE_BODY:
                m_index = 0;
                parse_body(buffer);
                break;
        }
        if (m_request.complete() || !good())
            return;
    }
}

void HttpRequestParser::process_headers()
{
    m_phase = PHASE_BODY;
    m_state = 0;
    if (m_message.version() > HttpMessage::HTTP_V10)
        process_host();
    process_transfer_encoding();
    process_content_length();
    if (route.action == ROUTE_UPLOAD)
        process_content_type();
    if (m_discard_body)
        m_message.setComplete(true);
    else if (route.action != ROUTE_UPLOAD)
    {
        if (m_message.body().open_file() < 0)
            return setError(error::bad_request);
    }
}

void HttpRequestParser::parse_headers(Buffer &buff)
{
    const static Handler handlers[6] = {
        &HttpParserState::hdr_start,
        &HttpParserState::hdr_name,
        &HttpParserState::hdr_space_before_value,
        &HttpParserState::hdr_value,
        &HttpParserState::hdr_almost_done,
        &HttpParserState::hdr_header_almost_done,
    };
    while (!buff.empty())
    {
        char ch = buff.getc();
        m_parsed++;
        if (m_parsed > MAX_HEADERS_BUFFER)
        {
            setError(error::header_too_large);
            return;
        }
        unsigned int action = (this->*handlers[m_state])(ch);

        switch (action)
        {
            case RES_HDR_ERROR:
                break;
            case RES_HEADER_DONE:
                process_headers();
                return;
            case RES_HEADER_LINE_DONE:
                process_header_line();
                break;
            case RES_HDR_CONTINUE:
                break;
        }
        if (!good() || m_message.complete())
            return;
    }
}

HttpRequestParser::~HttpRequestParser()
{
}
