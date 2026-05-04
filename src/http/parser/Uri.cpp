#include "Uri.hpp"
#include <cstddef>
#include <iostream>
#include <list>
#include <string>

Uri::Uri() : m_origin(), m_path(), m_query(), m_params(),m_isvalid(true)
{
}

Uri::Uri(const std::string &uri)
    : m_origin(uri),
      m_path(),
      m_query(),
      m_params(),
    m_isvalid(true)
{
    setUri(uri);
}

void Uri::setUri(const std::string &uri)
{
    std::string::size_type query_pos;
    std::string::size_type param_pos;
    std::string::size_type fragm_pos;

    std::string::size_type end_pos;

    // handle fragment
    fragm_pos = uri.find('#');
    end_pos   = uri.size();
    if (fragm_pos != std::string::npos)
        end_pos = fragm_pos;

    // handle query
    query_pos = uri.find('?');
    if (query_pos != std::string::npos && query_pos < end_pos)
    {
        m_query = uri.substr(query_pos + 1, end_pos - query_pos - 1);
        end_pos = query_pos;
    }

    // handle  param
    param_pos = uri.find(';');
    if (param_pos != std::string::npos && param_pos < end_pos)
    {
        m_params = uri.substr(param_pos + 1, end_pos - param_pos - 1);
        end_pos  = param_pos;
    }

    // handle path
    m_path = uri.substr(0, end_pos);
    process_path();
    process_params();
    process_query();
}

void Uri::process_path()
{
    std::list<std::string> segments;
    std::string            segment;

    for (size_t i = 0; i < m_path.size(); i++)
    {
        if (m_path[i] == '/')
        {
            segment += '/';
            if (segment == "../")
            {
                if (segments.size() > 0)
                    segments.pop_back();
            }
            else if (!segment.empty() && segment != "./" && segment != "/")
                segments.push_back(segment);
            segment.clear();
        }
        else if (m_path[i] == '%')
        {
            if (m_path.size() > i + 1 && m_path[i + 1] == '%')
            {
                segment += '%';
                i++;
                continue;
            }
            if (m_path.size() > i + 2 && isxdigit(m_path[i + 1]) &&
                isxdigit(m_path[i + 2]))
            {
                unsigned char hi = m_path[i + 1];
                unsigned char lo = m_path[i + 2];

                char c  = (hi >= 'a') ? hi - 'a' + 10
                        : (hi >= 'A') ? hi - 'A' + 10
                                      : hi - '0';
                c      *= 16;
                c      += (lo >= 'a') ? lo - 'a' + 10
                        : (lo >= 'A') ? lo - 'A' + 10
                                      : lo - '0';
                if (c == '\0' || c == '/')
                {
                    m_isvalid = false;
                    return;
                }
                segment += c;
                i       += 2;
                continue;
            }
            m_isvalid = false;
            return;
        }
        else
            segment += m_path[i];
    }
    if (!segment.empty() && segment != ".")
    {
        if (segment != "..")
            segments.push_back(segment);
        else
            segments.pop_back();
    }

    m_path                              = "/";
    std::list<std::string>::iterator it = segments.begin();
    while (it != segments.end())
    {
        m_path += *it;
        it++;
    }
}

const std::string Uri::path() const
{
    return m_path;
}

const std::string Uri::query() const
{
    return m_query;
}

const std::string Uri::params() const
{
    return m_params;
}

const std::string Uri::origin() const
{
    return m_params;
}
bool Uri::isvalid() const
{
    return m_isvalid;
}

void Uri::process_query()
{
}

void Uri::process_params()
{
}

Uri::~Uri()
{
}
