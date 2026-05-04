#include "HttpMessage.hpp"
#include "BodyStorage.hpp"
#include "Status.hpp"
#include <cstring>
#include <string>

HttpMessage::HttpMessage()
    : m_version(HTTP_V10),
      m_headers(),
      m_status(status::OK),
      m_content_length(0),
      m_body(),
      m_complete(false)
{
}

HttpMessage::HttpMessage(status::Status status)
    : m_version(HTTP_V10),
      m_headers(),
      m_status(status),
      m_content_length(0),
      m_body(),
      m_complete(true)
{
}

std::string
HttpMessage::extract_key(const std::string &name, const std::string &key)
{
    const_iterator it = getHeader(name);

    if (it == m_headers.end())
        return "";
    std::string content_type = it->second;

    std::size_t pos = content_type.find(key + "=");
    if (pos == std::string::npos)
        return "";

    pos += key.size() + 1; // skip "boundary="

    // boundary might be quoted: boundary="----WebKit..."
    if (pos < content_type.size() && content_type[pos] == '"')
    {
        ++pos; // skip opening "
        std::size_t end = content_type.find('"', pos);
        if (end == std::string::npos)
            return "";
        return content_type.substr(pos, end - pos);
    }

    // unquoted: boundary=----WebKit...
    // ends at ; or end of string
    std::size_t end = content_type.find(';', pos);
    if (end == std::string::npos)
        return content_type.substr(pos);

    return content_type.substr(pos, end - pos);
}

void HttpMessage::setHeader(const std::string &name, const std::string &value)
{
    m_headers.insert(Headers::value_type(name, value));
}

HttpMessage::const_iterator
HttpMessage::getHeader(const std::string &header_name) const
{
    return m_headers.find(header_name);
}

unsigned int HttpMessage::version() const
{
    return m_version;
}

void HttpMessage::setVersion(unsigned int major, unsigned int minor)
{
    m_version = major * 1000 + minor;
}

size_t HttpMessage::content_length() const
{
    return m_content_length;
}

void HttpMessage::setContentLength(size_t size)
{
    m_content_length = size;
}

BodyStorage &HttpMessage::body()
{
    return m_body;
}

const BodyStorage &HttpMessage::body() const
{
    return m_body;
}

bool HttpMessage::complete() const
{
    return m_complete;
}

void HttpMessage::setComplete(bool val)
{
    m_complete = good() && val;
}

unsigned int HttpMessage::version_major() const
{
    return m_version / 1000;
}

unsigned int HttpMessage::version_minor() const
{
    return m_version % 1000;
}

bool HttpMessage::good() const
{
    const StatusClass i = to_status_class(m_status);
    return i != status_class::CLIENT_ERROR && i != status_class::SERVER_ERROR;
}

Status HttpMessage::status() const
{
    return m_status;
}

void HttpMessage::setStatus(Status code)
{
    m_status = code;
}

HttpMessage::Headers &HttpMessage::headers()
{
    return m_headers;
}

const HttpMessage::Headers &HttpMessage::headers() const
{
    return m_headers;
}

void HttpMessage::clear()
{
    m_version = HTTP_V10;
    m_headers.clear();
    m_status         = status::OK;
    m_content_length = 0;
    m_body.clear();
    m_complete = false;
}

HttpMessage::~HttpMessage() {};
