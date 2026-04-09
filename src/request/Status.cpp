#include "Status.hpp"
#include <string>

const std::string phrase_reason(Status v)
{
    switch (v)
    {
        // 1xx
        case Status::continue_:
            return "Continue";
        case Status::switching_protocols:
            return "Switching Protocols";
        case Status::processing:
            return "Processing";
        case Status::early_hints:
            return "Early Hints";

        // 2xx
        case Status::ok:
            return "OK";
        case Status::created:
            return "Created";
        case Status::accepted:
            return "Accepted";
        case Status::non_authoritative_information:
            return "Non-Authoritative Information";
        case Status::no_content:
            return "No Content";
        case Status::reset_content:
            return "Reset Content";
        case Status::partial_content:
            return "Partial Content";
        case Status::multi_status:
            return "Multi-Status";
        case Status::already_reported:
            return "Already Reported";
        case Status::im_used:
            return "IM Used";

        // 3xx
        case Status::multiple_choices:
            return "Multiple Choices";
        case Status::moved_permanently:
            return "Moved Permanently";
        case Status::found:
            return "Found";
        case Status::see_other:
            return "See Other";
        case Status::not_modified:
            return "Not Modified";
        case Status::use_proxy:
            return "Use Proxy";
        case Status::temporary_redirect:
            return "Temporary Redirect";
        case Status::permanent_redirect:
            return "Permanent Redirect";

        // 4xx
        case Status::bad_request:
            return "Bad Request";
        case Status::unauthorized:
            return "Unauthorized";
        case Status::payment_required:
            return "Payment Required";
        case Status::forbidden:
            return "Forbidden";
        case Status::not_found:
            return "Not Found";
        case Status::method_not_allowed:
            return "Method Not Allowed";
        case Status::not_acceptable:
            return "Not Acceptable";
        case Status::proxy_authentication_required:
            return "Proxy Authentication Required";
        case Status::request_timeout:
            return "Request Timeout";
        case Status::conflict:
            return "Conflict";
        case Status::gone:
            return "Gone";
        case Status::length_required:
            return "Length Required";
        case Status::precondition_failed:
            return "Precondition Failed";
        case Status::payload_too_large:
            return "Payload Too Large";
        case Status::uri_too_long:
            return "URI Too Long";
        case Status::unsupported_media_type:
            return "Unsupported Media Type";
        case Status::range_not_satisfiable:
            return "Range Not Satisfiable";
        case Status::expectation_failed:
            return "Expectation Failed";
        case Status::i_am_a_teapot:
            return "I'm a teapot";
        case Status::misdirected_request:
            return "Misdirected Request";
        case Status::unprocessable_entity:
            return "Unprocessable Entity";
        case Status::locked:
            return "Locked";
        case Status::failed_dependency:
            return "Failed Dependency";
        case Status::too_early:
            return "Too Early";
        case Status::upgrade_required:
            return "Upgrade Required";
        case Status::precondition_required:
            return "Precondition Required";
        case Status::too_many_requests:
            return "Too Many Requests";
        case Status::request_header_fields_too_large:
            return "Request Header Fields Too Large";
        case Status::unavailable_for_legal_reasons:
            return "Unavailable For Legal Reasons";
        // 5xx
        case Status::internal_server_error:
            return "Internal Server Error";
        case Status::not_implemented:
            return "Not Implemented";
        case Status::bad_gateway:
            return "Bad Gateway";
        case Status::service_unavailable:
            return "Service Unavailable";
        case Status::gateway_timeout:
            return "Gateway Timeout";
        case Status::http_version_not_supported:
            return "HTTP Version Not Supported";
        case Status::variant_also_negotiates:
            return "Variant Also Negotiates";
        case Status::insufficient_storage:
            return "Insufficient Storage";
        case Status::loop_detected:
            return "Loop Detected";
        case Status::not_extended:
            return "Not Extended";
        case Status::network_authentication_required:
            return "Network Authentication Required";

        default:
            break;
    }
    return "<unknown-status>";
}

StatusClass to_status_class(unsigned v)
{
    switch (v / 100)
    {
        case 1:
            return status_class::informational;
        case 2:
            return status_class::successful;
        case 3:
            return status_class::redirection;
        case 4:
            return status_class::client_error;
        case 5:
            return status_class::server_error;
        default:
            break;
    }
    return status_class::unknown;
}

std::ostream &operator<<(std::ostream &os, Status v)
{
    return os << phrase_reason(v);
}
