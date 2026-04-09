#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "Method.hpp"
#include "Status.hpp"
#include <fstream>

class HttpMessage
{
    const unsigned int HTTP_V11 = 1001;
    const unsigned int HTTP_V10 = 1000;
    const unsigned int HTTP_V09 = 9;

  protected:
    unsigned int  m_version;
    Method        m_method;
    size_t        m_content_length;
    Status        m_status;
    std::string   m_body;
    std::ofstream m_body_file;

  public:
    HttpMessage();
    HttpMessage(const HttpMessage &other);

    virtual ~HttpMessage()          = 0;
    virtual bool incomplete() const = 0;

    HttpMessage &operator=(const HttpMessage &other);

    bool good() const;

    unsigned int version_major() const;
    unsigned int version_minor() const;

    void   setMethod(Method method);
    Method method() const;

    void         setVersion(unsigned int major, unsigned int minor = 0);
    unsigned int version() const;

    size_t content_length() const;
    void   setContentLength(size_t size);

    const std::string &body_file_name() const;
    std::string       &body_file_name();

    Status         status() const;
    void           setStatus(Status code);
    std::ofstream &body_file_ostream();
};
#endif
