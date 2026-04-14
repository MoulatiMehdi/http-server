#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include "Status.hpp"
#include <iostream>
#include <string>
#include <sys/types.h>

HttpParserState::HttpParserState(HttpRequest &request)
    : m_state(0),
      m_chunked(false),
      m_phase(P_REQUEST_LINE),
      m_error(error::ok),
      request(request)
{
}

void HttpParserState::processError(HttpRequest &request)
{
    switch (m_error)
    {
        case error::ok:
            return request.setStatus(status::OK);
        case error::bad_request:
        case error::bad_line_ending:
        case error::bad_target:
        case error::bad_version:
        case error::bad_method:
        case error::bad_status:
        case error::bad_reason:
        case error::bad_field:
        case error::bad_header_name:
        case error::bad_header_value:
        case error::bad_content_length:
        case error::multiple_content_length:
        case error::bad_value:
            return request.setStatus(status::BAD_REQUEST);
        case error::unsupported_version:
            return request.setStatus(status::HTTP_VERSION_NOT_SUPPORTED);
        case error::unsupported_method:
            return request.setStatus(status::METHOD_NOT_ALLOWED);
        case error::unsupported_transfer:
            return request.setStatus(status::NOT_IMPLEMENTED);
        case error::header_field_name_too_large:
        case error::header_field_value_too_large:
            return request.setStatus(status::REQUEST_HEADER_FIELDS_TOO_LARGE);
        case error::stale_parser:
            return request.setStatus(status::BAD_REQUEST);
        case error::short_read:
            break;
    }
}

void HttpParserState::setError(ParserError err)
{
    if (m_error == error::ok)
    {
        m_error = err;
        processError(request);
    }
}

bool HttpParserState::good() const
{
    return m_error == error::ok;
}

std::ostream &operator<<(std::ostream &os, const HttpParserState &hps)
{
    std::string phase[3] = {"Request Line", "Headers", "Body"};

    std::cout << "Error      : " << to_string(hps.m_error) << std::endl;
    if (!hps.request.complete())
        std::cout << "Phase      : " << phase[hps.m_phase] << std::endl;
    else
        std::cout << "Phase      : Complete" << std::endl;
    if (!hps.good())
        std::cout << "State      : " << hps.m_state << std::endl;

    return os;
}

HttpParserState::~HttpParserState()
{
}
