#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include "BodyStorage.hpp"
#include "Status.hpp"
#include <cstddef>
#include <map>
#include <string>

class HttpMessage
{
  public:
    struct iless
    {
        typedef unsigned int Int;

        static inline Int get_chars(const unsigned char *p)
        {
            return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
        }

        bool operator()(const std::string &lhs, const std::string &rhs) const
        {
            std::size_t n1 = lhs.size();
            std::size_t n2 = rhs.size();

            std::size_t n = n1 < n2 ? n1 : n2;

            const unsigned char *p1 =
                reinterpret_cast<const unsigned char *>(lhs.data());
            const unsigned char *p2 =
                reinterpret_cast<const unsigned char *>(rhs.data());

            const int S    = sizeof(Int);
            const Int Mask = 0xDFDFDFDF;

            for (; n >= S; p1 += S, p2 += S, n -= S)
            {
                const Int v1 = get_chars(p1) & Mask;
                const Int v2 = get_chars(p2) & Mask;
                if (v1 < v2)
                    return true;
                if (v1 > v2)
                    return false;
            }
            for (; n; ++p1, ++p2, --n)
            {
                const unsigned char c1 = *p1 & 0xDF;
                const unsigned char c2 = *p2 & 0xDF;
                if (c1 < c2)
                    return true;
                if (c1 > c2)
                    return false;
            }
            return n1 < n2;
        }
    };

    typedef std::multimap<const std::string, std::string, iless> Headers;
    typedef std::pair<Headers::const_iterator, Headers::const_iterator>
                                    HeadersRange;
    typedef Headers::const_iterator const_iterator;
    typedef Headers::iterator       iterator;

    static const unsigned int HTTP_V11 = 1001;
    static const unsigned int HTTP_V10 = 1000;
    static const unsigned int HTTP_V09 = 9;

  private:
  protected:
    unsigned int m_version;
    Headers      m_headers;
    Status       m_status;
    ssize_t      m_content_length;
    BodyStorage  m_body;
    bool         m_complete;

  public:
    HttpMessage();
    HttpMessage(status::Status status);
    virtual ~HttpMessage() = 0;

    bool complete() const;
    bool good() const;

    unsigned int   version_major() const;
    unsigned int   version_minor() const;
    unsigned int   version() const;
    size_t         content_length() const;
    Status         status() const;
    const_iterator getHeader(const std::string &name) const;
    const Headers &headers() const;

    Headers           &headers();
    BodyStorage       &body();
    const BodyStorage &body() const;

    void setComplete(bool val);
    void setVersion(unsigned int major, unsigned int minor = 0);
    void setContentLength(size_t size);
    void setStatus(Status code);
    void setHeader(const std::string &name, const std::string &value);

    void        clear();
    std::string extract_key(const std::string &name, const std::string &key);
};
#endif
