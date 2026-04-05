#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

namespace error
{

    enum Error
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
        unsupported_schema,
        stale_parser,
        short_read,
        header_field_name_too_large,
        header_field_value_too_large
    };
} // namespace error

//
using Error = error::Error;
std::string   to_string(Error error);
std::ostream &operator<<(std::ostream &os, const Error &error);
#endif
