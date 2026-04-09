#include "Status.hpp"
#include <string>

const std::string phrase_reason(Status v)
{
    switch (v)
    {
        // 1xx
        case Status::CONTINUE:
            return "Continue";
        case Status::SWITCHING_PROTOCOLS:
            return "Switching Protocols";
        case Status::PROCESSING:
            return "Processing";
        case Status::EARLY_HINTS:
            return "Early Hints";

        // 2xx
        case Status::OK:
            return "OK";
        case Status::CREATED:
            return "Created";
        case Status::ACCEPTED:
            return "Accepted";
        case Status::NON_AUTHORITATIVE_INFORMATION:
            return "Non-Authoritative Information";
        case Status::NO_CONTENT:
            return "No Content";
        case Status::RESET_CONTENT:
            return "Reset Content";
        case Status::PARTIAL_CONTENT:
            return "Partial Content";
        case Status::MULTI_STATUS:
            return "Multi-Status";
        case Status::ALREADY_REPORTED:
            return "Already Reported";
        case Status::IM_USED:
            return "IM Used";

        // 3xx
        case Status::MULTIPLE_CHOICES:
            return "Multiple Choices";
        case Status::MOVED_PERMANENTLY:
            return "Moved Permanently";
        case Status::FOUND:
            return "Found";
        case Status::SEE_OTHER:
            return "See Other";
        case Status::NOT_MODIFIED:
            return "Not Modified";
        case Status::USE_PROXY:
            return "Use Proxy";
        case Status::TEMPORARY_REDIRECT:
            return "Temporary Redirect";
        case Status::PERMANENT_REDIRECT:
            return "Permanent Redirect";

        // 4xx
        case Status::BAD_REQUEST:
            return "Bad Request";
        case Status::UNAUTHORIZED:
            return "Unauthorized";
        case Status::PAYMENT_REQUIRED:
            return "Payment Required";
        case Status::FORBIDDEN:
            return "Forbidden";
        case Status::NOT_FOUND:
            return "Not Found";
        case Status::METHOD_NOT_ALLOWED:
            return "Method Not Allowed";
        case Status::NOT_ACCEPTABLE:
            return "Not Acceptable";
        case Status::PROXY_AUTHENTICATION_REQUIRED:
            return "Proxy Authentication Required";
        case Status::REQUEST_TIMEOUT:
            return "Request Timeout";
        case Status::CONFLICT:
            return "Conflict";
        case Status::GONE:
            return "Gone";
        case Status::LENGTH_REQUIRED:
            return "Length Required";
        case Status::PRECONDITION_FAILED:
            return "Precondition Failed";
        case Status::PAYLOAD_TOO_LARGE:
            return "Payload Too Large";
        case Status::URI_TOO_LONG:
            return "URI Too Long";
        case Status::UNSUPPORTED_MEDIA_TYPE:
            return "Unsupported Media Type";
        case Status::RANGE_NOT_SATISFIABLE:
            return "Range Not Satisfiable";
        case Status::EXPECTATION_FAILED:
            return "Expectation Failed";
        case Status::I_AM_A_TEAPOT:
            return "I'm a teapot";
        case Status::MISDIRECTED_REQUEST:
            return "Misdirected Request";
        case Status::UNPROCESSABLE_ENTITY:
            return "Unprocessable Entity";
        case Status::LOCKED:
            return "Locked";
        case Status::FAILED_DEPENDENCY:
            return "Failed Dependency";
        case Status::TOO_EARLY:
            return "Too Early";
        case Status::UPGRADE_REQUIRED:
            return "Upgrade Required";
        case Status::PRECONDITION_REQUIRED:
            return "Precondition Required";
        case Status::TOO_MANY_REQUESTS:
            return "Too Many Requests";
        case Status::REQUEST_HEADER_FIELDS_TOO_LARGE:
            return "Request Header Fields Too Large";
        case Status::UNAVAILABLE_FOR_LEGAL_REASONS:
            return "Unavailable For Legal Reasons";
        // 5xx
        case Status::INTERNAL_SERVER_ERROR:
            return "Internal Server Error";
        case Status::NOT_IMPLEMENTED:
            return "Not Implemented";
        case Status::BAD_GATEWAY:
            return "Bad Gateway";
        case Status::SERVICE_UNAVAILABLE:
            return "Service Unavailable";
        case Status::GATEWAY_TIMEOUT:
            return "Gateway Timeout";
        case Status::HTTP_VERSION_NOT_SUPPORTED:
            return "HTTP Version Not Supported";
        case Status::VARIANT_ALSO_NEGOTIATES:
            return "Variant Also Negotiates";
        case Status::INSUFFICIENT_STORAGE:
            return "Insufficient Storage";
        case Status::LOOP_DETECTED:
            return "Loop Detected";
        case Status::NOT_EXTENDED:
            return "Not Extended";
        case Status::NETWORK_AUTHENTICATION_REQUIRED:
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
            return StatusClass::INFORMATIONAL;
        case 2:
            return StatusClass::SUCCESSFUL;
        case 3:
            return StatusClass::REDIRECTION;
        case 4:
            return StatusClass::CLIENT_ERROR;
        case 5:
            return StatusClass::SERVER_ERROR;
        default:
            break;
    }
    return StatusClass::UNKNOWN;
}

std::ostream &operator<<(std::ostream &os, Status v)
{
    return os << phrase_reason(v);
}
