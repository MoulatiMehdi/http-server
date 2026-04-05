
#include "State.hpp"
#include <string>

const std::string to_string(state::State s)
{
    switch (s)
    {
        case state::s_body_start:
            return "s_body_start";
        case state::s_req_start:
            return "sw_start = 0";

        case state::s_req_method:
            return "sw_method";

        case state::s_req_spaces_before_uri:
            return "sw_spaces_before_uri";

        case state::s_req_after_slash_in_uri:
            return "sw_after_slash_in_uri";

        case state::s_req_check_uri:
            return "sw_check_uri";

        case state::s_req_uri:
            return "sw_uri";

        case state::s_req_http_09:
            return "sw_http_09";

        case state::s_req_http_H:
            return "s_req_http_H";

        case state::s_req_http_HT:
            return "s_req_http_HT";

        case state::s_req_http_HTT:
            return "s_req_http_HTT";

        case state::s_req_http_HTTP:
            return "sw_http_HTTP";

        case state::s_req_first_major_digit:
            return "sw_first_major_digit";

        case state::s_req_major_digit:
            return "sw_major_digit";

        case state::s_req_first_minor_digit:
            return "sw_first_minor_digit";

        case state::s_req_minor_digit:
            return "sw_minor_digit";

        case state::s_req_spaces_after_digit:
            return "sw_spaces_after_digit";

        case state::s_req_almost_done:
            return "sw_spaces_after_digit";
        case state::s_req_done:
            return "sw_req_done";

        case state::s_hdr_start:
            return "s_hdr_start";
        case state::s_hdr_name:
            return "s_hdr_name";
        case state::s_hdr_space_before_value:
            return "s_hdr_space_before_value";
        case state::s_hdr_value:
            return "s_hdr_value";
        case state::s_hdr_almost_done:
            return "s_hdr_almost_done";
        case state::s_hdr_line_done:
            return "s_hdr_done";
        case state::s_hdr_header_almost_done:
            return "s_hdr_header_almost_done";
        case state::s_hdr_done:
            return "s_hdr_done";
            // case state::s_body_start:
            //     return "s_body_start";
    }
    return "Unknown";
}
