#include "HttpMessage.hpp"
#include "Method.hpp"
#include "Status.hpp"
#include <bits/types/struct_timeval.h>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/time.h>

static const std::string generate_name()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    std::ostringstream iss;

    iss << "/tmp/nginx/" << tv.tv_sec << "." << tv.tv_usec;
    return iss.str();
}

HttpMessage::HttpMessage()
    : m_method(method::Unknown),
      m_version(HTTP_V10),
      m_content_length(0),
      m_status(Status::ok),
      m_body(generate_name()),
      m_body_file(m_body)
{
}

HttpMessage::HttpMessage(const HttpMessage &other)
    : m_method(other.m_method),
      m_version(other.m_version),
      m_content_length(other.m_content_length),
      m_status(other.m_status),
      m_body(generate_name()),
      m_body_file(m_body)
{
}

HttpMessage &HttpMessage::operator=(const HttpMessage &other)
{
    m_method         = other.m_method;
    m_version        = other.m_version;
    m_status         = other.m_status;
    m_content_length = other.m_content_length;
    m_body           = generate_name();
    m_body_file.open(m_body);
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

void HttpMessage::setVersion(unsigned int major, unsigned int minor)
{
    m_version = major * 1000 + minor;
}

void HttpMessage::setMethod(Method method)
{
    m_method = method;
}

size_t HttpMessage::content_length() const
{
    return m_content_length;
}

void HttpMessage::setContentLength(size_t size)
{
    m_content_length = size;
}

const std::string &HttpMessage::body_file_name() const
{
    return m_body;
}

std::string &HttpMessage::body_file_name()
{
    return m_body;
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
    return m_status == Status::ok;
}

Status HttpMessage::status() const
{
    return m_status;
}

void HttpMessage::setStatus(Status code)
{
    m_status = code;
}

std::ofstream &HttpMessage::body_file_ostream()
{
    return m_body_file;
}

HttpMessage::~HttpMessage()
{
    m_body_file.close();
    std::remove(m_body.c_str());
};
