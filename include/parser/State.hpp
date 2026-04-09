#ifndef STATE_HPP
#define STATE_HPP

#include <string>

namespace state
{

    enum State
    {
        s_req_start = 0,
        s_req_method,
        s_req_spaces_before_uri,
        s_req_uri_after_slash,
        s_req_check_uri,
        s_req_uri,
        s_req_http_09,
        s_req_http_H,
        s_req_http_HT,
        s_req_http_HTT,
        s_req_http_HTTP,
        s_req_first_major_digit,
        s_req_major_digit,
        s_req_first_minor_digit,
        s_req_minor_digit,
        s_req_spaces_after_digit,
        s_req_almost_done,
        s_hdr_start,
        s_hdr_name,
        s_hdr_space_before_value,
        s_hdr_value,
        s_hdr_almost_done,
        s_hdr_header_almost_done,
        s_body_start,
    };
} // namespace state

using State = state::State;
const std::string to_string(State state);
#endif
