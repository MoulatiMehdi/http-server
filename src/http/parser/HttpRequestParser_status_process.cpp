
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "Status.hpp"
#include <cerrno>

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
        case error::url_too_large:
            return m_request.setStatus(status::URI_TOO_LONG);
        case error::body_too_large:
            return m_request.setStatus(status::PAYLOAD_TOO_LARGE);
            break;
        case error::bad_upload:
            switch (errno)
            {
                // Disk / storage issues
                case ENOSPC: // No space left on device
                case EDQUOT: // Disk quota exceeded
                    m_request.setStatus(
                        status::INSUFFICIENT_STORAGE
                    );       // Insufficient Storage

                // Permission issues
                case EACCES:                                // Permission denied
                case EPERM:
                    m_request.setStatus(status::FORBIDDEN); // Forbidden

                // File already exists
                case EEXIST:
                    m_request.setStatus(status::CONFLICT); // Conflict

                // Bad input / invalid path
                case EINVAL:
                case ENAMETOOLONG:
                case EISDIR:
                    m_request.setStatus(status::BAD_REQUEST); // Bad Request

                // Default fallback
                default:
                    m_request.setStatus(status::INTERNAL_SERVER_ERROR);
            }
            break;
        case error::unsupported_media_type:
            m_request.setStatus(status::UNSUPPORTED_MEDIA_TYPE);
            break;
    }
}
