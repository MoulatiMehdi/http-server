
#include "HttpRequestParser.hpp"

void HttpRequestParser::process_error()
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
