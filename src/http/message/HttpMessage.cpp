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
    m_complete = m_status == status::OK && val;
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

HttpMessage::~HttpMessage() {};
