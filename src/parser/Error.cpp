
#include "ParserError.hpp"
#include <string>

std::string to_string(ParserError err)
{
    switch (err)
    {
        case ParserError::bad_target:
            return "Bad Target";
        case ParserError::bad_method:
            return "Bad Method";
        case ParserError::bad_request:
            return "Bad Request";
        case ParserError::bad_version:
            return "Bad Version";
        case ParserError::bad_line_ending:
            return "Bad Line Ending";
        case ParserError::unsupported_version:
            return "Unsupported Version";
        case ParserError::unsupported_method:
            return "Unsupported Method";
        case ParserError::unsupported_transfer:
            return "Unsupported transfer";
        case ParserError::bad_status:
            return "Bad Status";
        case ParserError::bad_field:
            return "Bad Field";
        case ParserError::bad_reason:
            return "Bad Reason";
        case ParserError::bad_value:
            return "Bad value";
        case ParserError::bad_content_length:
            return "Bad content length";
        case ParserError::multiple_content_length:
            return "multiple_content_length";
        case ParserError::stale_parser:
            return "stale parser";
        case ParserError::short_read:
            return "bad_transfer_encoding";
        case ParserError::header_field_name_too_large:
            return "header_field_name_too_large";
        case ParserError::header_field_value_too_large:
            return "header_field_value_too_large";
        case ParserError::bad_header_name:
            return "bad_header_name";
        case ParserError::bad_header_value:
            return "bad_header_value";
        case ParserError::ok:
            return "OK";
    }
}
