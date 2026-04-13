#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "BodyStorage.hpp"
#include "Status.hpp"
#include <map>

class HttpMessage
{
  public:
    typedef std::multimap<const std::string, std::string> Headers;
    typedef Headers::const_iterator                       const_iterator;

  protected:
    unsigned int m_version;
    Headers      m_headers;
    Status       m_status;
    ssize_t       m_content_length;
    BodyStorage  m_body;

  public:
    static const unsigned int HTTP_V11 = 1001;
    static const unsigned int HTTP_V10 = 1000;
    static const unsigned int HTTP_V09 = 9;

    HttpMessage();
    // HttpMessage &operator=(const HttpMessage &other);
    // HttpMessage(const HttpMessage &other);

    virtual ~HttpMessage() = 0;

    virtual bool complete() const = 0;
    bool         good() const;

    unsigned int   version_major() const;
    unsigned int   version_minor() const;
    unsigned int   version() const;
    ssize_t         content_length() const;
    Status         status() const;
    const_iterator getHeader(const std::string &name) const;
    const Headers &headers() const;

    Headers           &headers();
    BodyStorage       &body();
    const BodyStorage &body() const;

    void setVersion(unsigned int major, unsigned int minor = 0);
    void setContentLength(size_t size);
    void setStatus(Status code);
    void setHeader(const std::string &name, const std::string &value);
    void clear();

};
#endif
