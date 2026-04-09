#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "Method.hpp"
#include "Status.hpp"
#include <fstream>

class HttpMessage
{
  protected:
    unsigned int  m_version;
    Method        m_method;
    size_t        m_content_length;
    Status        m_status;
    std::string   m_body_file_name;
    std::ofstream m_body_file_ostream;

  public:
    static const unsigned int HTTP_V11 = 1001;
    static const unsigned int HTTP_V10 = 1000;
    static const unsigned int HTTP_V09 = 9;

    HttpMessage();
    HttpMessage(const HttpMessage &other);

    virtual ~HttpMessage() = 0;

    virtual bool complete() const = 0;
    bool         good() const;

    unsigned int version_major() const;
    unsigned int version_minor() const;
    unsigned int version() const;
    Method       method() const;
    size_t       content_length() const;
    Status       status() const;

    void setVersion(unsigned int major, unsigned int minor = 0);
    void setMethod(Method method);
    void setContentLength(size_t size);
    void setStatus(Status code);

    HttpMessage &operator=(const HttpMessage &other);

    const std::string &body_file_name() const;
    std::string       &body_file_name();

    std::ofstream &body_file_ostream();
};
#endif
