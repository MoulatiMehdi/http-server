#include "HttpMessage.hpp"
#include "HttpMessage.hpp"
#include "Method.hpp"

HttpMessage::HttpMessage() : m_method(method::Unknown), m_version(1000)
{
}

HttpMessage::HttpMessage(const HttpMessage &other)
    : m_method(other.m_method),
      m_version(other.m_version)
{
}

HttpMessage &HttpMessage::operator=(const HttpMessage &other)
{
    m_method  = other.m_method;
    m_version = other.m_version;
    return *this;
}

unsigned int HttpMessage::version() const
{
    return m_version;
}

Method HttpMessage::method() const
{
    return m_method;
}

void HttpMessage::set_version(unsigned int major, unsigned int minor)
{
    m_version = major * 1000 + minor;
}

void HttpMessage::set_method(Method method)
{
    m_method = method;
}

unsigned int HttpMessage::version_major() const
{
    return m_version / 1000;
}

unsigned int HttpMessage::version_minor() const
{
    return m_version % 1000;
}

HttpMessage::~HttpMessage() {};
