#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "Method.hpp"

class HttpMessage
{
    const unsigned int HTTP_V11 = 1001;
    const unsigned int HTTP_V10 = 1000;
    const unsigned int HTTP_V09 = 9;

  protected:
    unsigned int m_version;
    Method       m_method;

  public:
    HttpMessage();
    HttpMessage(const HttpMessage &other);

    virtual ~HttpMessage() = 0;

    HttpMessage &operator=(const HttpMessage &other);

    Method       method() const;
    unsigned int version() const;
    unsigned int version_major() const;
    unsigned int version_minor() const;
    bool         good() const;

    void set_version(unsigned int major, unsigned int minor = 0);
    void set_method(Method method);
};
#endif
