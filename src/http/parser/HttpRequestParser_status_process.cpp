
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "Status.hpp"

void HttpRequestParser::process_error()
{
    switch (m_error)
    {
        case error::ok:
            return m_request.setStatus(status::OK);
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
            return m_request.setStatus(status::BAD_REQUEST);
        case error::unsupported_version:
            return m_request.setStatus(status::HTTP_VERSION_NOT_SUPPORTED);
        case error::unsupported_method:
            return m_request.setStatus(status::METHOD_NOT_ALLOWED);
        case error::unsupported_transfer:
            return m_request.setStatus(status::NOT_IMPLEMENTED);
        case error::header_too_large:
            return m_request.setStatus(status::REQUEST_HEADER_FIELDS_TOO_LARGE);
        case error::stale_parser:
            return m_request.setStatus(status::BAD_REQUEST);
        case error::short_read:
            break;
        case error::url_too_large:
            return m_request.setStatus(status::URI_TOO_LONG);
        case error::body_too_large:
            return m_request.setStatus(status::PAYLOAD_TOO_LARGE);
            break;
    }
}
