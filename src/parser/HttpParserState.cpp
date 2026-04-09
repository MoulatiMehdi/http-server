#include "HttpParserState.hpp"
#include "Error.hpp"
#include "HttpRequest.hpp"

HttpParserState::HttpParserState()
    : m_invalid_header(false),
      m_error(error::ok),
      m_state(state::s_req_start),
      m_complete(false),
      m_chunked(false),
      m_discard_body(true)
{
}

HttpParserState::HttpParserState(HttpParserState &)
{
}

HttpParserState &HttpParserState::operator=(HttpParserState &)
{
    return *this;
}

void HttpParserState::processError(HttpRequest &request)
{
    switch (m_error)
    {
        case error::ok:
            return request.setStatus(Status::OK);
        case Error::bad_request:
        case Error::bad_line_ending:
        case Error::bad_target:
        case Error::bad_version:
        case Error::bad_method:
        case Error::bad_status:
        case Error::bad_reason:
        case Error::bad_field:
        case Error::bad_header_name:
        case Error::bad_header_value:
        case Error::bad_content_length:
        case Error::multiple_content_length:
        case Error::bad_value:
            return request.setStatus(Status::BAD_REQUEST);
        case Error::unsupported_version:
            return request.setStatus(Status::HTTP_VERSION_NOT_SUPPORTED);
        case Error::unsupported_method:
            return request.setStatus(Status::METHOD_NOT_ALLOWED);
        case Error::unsupported_schema:
        case Error::unsupported_transfer:
            return request.setStatus(Status::NOT_IMPLEMENTED);
        case Error::header_field_name_too_large:
        case Error::header_field_value_too_large:
            return request.setStatus(Status::REQUEST_HEADER_FIELDS_TOO_LARGE);
        case Error::stale_parser:
        case Error::short_read:
            break;
    }
}

void HttpParserState::setError(Error err)
{
    if (m_error == error::ok)
        m_error = err;
}

Error HttpParserState::error() const
{
    return m_error;
}

State HttpParserState::state() const
{

    return m_state;
}

bool HttpParserState::good() const
{
    return m_error == error::ok;
}

HttpParserState::~HttpParserState()
{
}
