#ifndef PARSER_ERROR_HPP
#define PARSER_ERROR_HPP

#include <string>

namespace error
{

    enum ParserError
    {
        ok = 0,
        bad_request,
        bad_line_ending,
        bad_method,
        bad_target,
        bad_version,
        bad_status,
        bad_reason,
        bad_field,
        bad_header_name,
        bad_header_value,
        bad_value,
        bad_content_length,
        multiple_content_length,
        unsupported_version,
        unsupported_method,
        unsupported_transfer,
        url_too_large,
        header_too_large,
        body_too_large,
    };
} // namespace error

//
using error::ParserError;
std::string   to_string(ParserError error);
std::ostream &operator<<(std::ostream &os, const ParserError &error);
#endif
