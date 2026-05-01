#include "HttpParserState.hpp"
#include "HttpMessage.hpp"
#include "ParserError.hpp"
#include <iostream>
#include <string>
#include <sys/types.h>

HttpParserState::HttpParserState(HttpMessage &request)
    : m_parsed(0),
      m_state(0),
      m_chunked(false),
      m_discard_body(true),
      m_phase(PHASE_REQUEST_LINE),
      m_error(error::ok),
      request(request)
{
    m_major = 0;
    m_minor = 0;
}

void HttpParserState::setError(ParserError err)
{
    if (m_error == error::ok)
    {
        m_error = err;
        process_error();
    }
}

bool HttpParserState::good() const
{
    return m_error == error::ok;
}

std::ostream &operator<<(std::ostream &os, const HttpParserState &hps)
{
    std::string phase[3]     = {"Request Line", "Headers", "Body"};
    std::string state[3][20] = {
        {
         "SW_START", "SW_METHOD",
         "SW_SPACES_BEFORE_URI", "SW_URI_AFTER_SLASH",
         "SW_CHECK_URI", "SW_URI",
         "SW_HTTP_09", "SW_HTTP_H",
         "SW_HTTP_HT", "SW_HTTP_HTT",
         "SW_HTTP_HTTP", "SW_FIRST_MAJOR_DIGIT",
         "SW_MAJOR_DIGIT", "SW_FIRST_MINOR_DIGIT",
         "SW_MINOR_DIGIT", "SW_SPACES_AFTER_DIGIT",
         "SW_ALMOST_DONE", },
        {
         "SW_START", "SW_NAME",
         "SW_SPACE_BEFORE_VALUE", "SW_VALUE",
         "SW_ALMOST_DONE", "SW_HEADER_ALMOST_DONE",
         },
        {
         "SW_CHUNK_START", "SW_CHUNK_SIZE",
         "SW_CHUNK_SIZE_ALMOST_DONE", "SW_CHUNK_DATA",
         "SW_AFTER_DATA", "SW_AFTER_DATA_ALMOST_DONE",
         "SW_LAST_CHUNK_SIZE_ALMOST_DONE", "SW_LAST_CHUNK_DATA_ALMOST_DONE",
         "SW_BODY_ALMOST_DONE", }
    };

    std::cout << "Error      : " << to_string(hps.m_error) << std::endl;
    if (!hps.good())
        std::cout << "State      : " << state[hps.m_phase][hps.m_state]
                  << std::endl;
    else if (!hps.request.complete())
        std::cout << "Phase      : " << phase[hps.m_phase] << std::endl;
    else
        std::cout << "Phase      : Complete" << std::endl;

    return os;
}

void HttpParserState::parse_headers(Buffer &buff)
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
            case RES_ERROR:
                break;
            case RES_HEADER_DONE:
                process_headers(request);
                return;
            case RES_HEADER_LINE_DONE:
                process_header_line(request);
                break;
            case RES_CONTINUE:
                break;
        }
        if (!good() || request.complete())
            return;
    }
}

HttpParserState::~HttpParserState()
{
}
