
#include "ParserError.hpp"
#include <string>

std::string to_string(ParserError err)
{
    switch (err)
    {
        case error::bad_target:
            return "Bad Target";
        case error::bad_method:
            return "Bad Method";
        case error::bad_request:
            return "Bad Request";
        case error::bad_version:
            return "Bad Version";
        case error::bad_line_ending:
            return "Bad Line Ending";
        case error::unsupported_version:
            return "Unsupported Version";
        case error::unsupported_method:
            return "Unsupported Method";
        case error::unsupported_transfer:
            return "Unsupported transfer";
        case error::bad_status:
            return "Bad Status";
        case error::bad_field:
            return "Bad Field";
        case error::bad_reason:
            return "Bad Reason";
        case error::bad_value:
            return "Bad value";
        case error::bad_content_length:
            return "Bad content length";
        case error::multiple_content_length:
            return "multiple_content_length";
        case error::stale_parser:
            return "stale parser";
        case error::short_read:
            return "bad_transfer_encoding";
        case error::header_too_large:
            return "header_too_large";
        case error::bad_header_name:
            return "bad_header_name";
        case error::bad_header_value:
            return "bad_header_value";
        case error::ok:
            return "OK";
        case error::url_too_large:
            return "url_too_large";
        case error::body_too_large:
            return "body_too_large";
            break;
    }
}
