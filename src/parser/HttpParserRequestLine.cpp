#include "HttpParserRequestLine.hpp"
#include "Error.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "Method.hpp"

HttpParserRequestLine::HttpParserRequestLine()
    : HttpParserState(),
      m_major(0),
      m_minor(0),
      m_method(),
      m_target()
{
}

void HttpParserRequestLine::processRequestLine(HttpRequest &request)
{
    Method method = string_to_method(m_method);

    if (method == Method::Unknown)
    {
        setError(error::unsupported_method);
        return;
    }

    request.setMethod(method);
    request.setTarget(m_target);
    request.setVersion(m_major, m_minor);
    if (request.version() == 9)
    {
        setError(error::unsupported_version);
        return;
    }
    m_discard_body = request.method() != method::Post;
}

HttpParserRequestLine::~HttpParserRequestLine()
{
}
