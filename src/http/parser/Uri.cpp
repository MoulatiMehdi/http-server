#include "Uri.hpp"
#include <cstddef>
#include <iostream>
#include <list>
#include <string>

Uri::Uri(const std::string &uri) : isvalid(true)
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
        query   = uri.substr(query_pos + 1, end_pos - query_pos - 1);
        end_pos = query_pos;
    }

    // handle  param
    param_pos = uri.find(';');
    if (param_pos != std::string::npos && param_pos < end_pos)
    {
        params  = uri.substr(param_pos + 1, end_pos - param_pos - 1);
        end_pos = param_pos;
    }

    // handle path
    path = uri.substr(0, end_pos);
    process_path();
    process_params();
    process_query();
}

void Uri::process_path()
{
    std::list<std::string> segments;
    std::string            segment;

    segments.push_back("/");

    for (size_t i = 0; i < path.size(); i++)
    {
        std::cerr << segment << std::endl;
        if (path[i] == '/')
        {
            segment += '/';
            if (segment == "../")
            {
                if (segments.size() > 0)
                    segments.pop_back();
            }
            else if (!segment.empty() && segment != "./")
                segments.push_back(segment);
            segment.clear();
        }
        else if (path[i] == '%')
        {
            if (path.size() > i + 1 && path[i + 1] == '%')
            {
                segment += '%';
                i++;
                continue;
            }
            if (path.size() > i + 2 && isxdigit(path[i + 1]) &&
                isxdigit(path[i + 2]))
            {
                unsigned char hi = path[i + 1];
                unsigned char lo = path[i + 2];

                char c  = (hi >= 'a') ? hi - 'a' + 10
                        : (hi >= 'A') ? hi - 'A' + 10
                                      : hi - '0';
                c      *= 16;
                c      += (lo >= 'a') ? lo - 'a' + 10
                        : (lo >= 'A') ? lo - 'A' + 10
                                      : lo - '0';
                if (c == '\0' || c == '/')
                {
                    isvalid = false;
                    return;
                }
                segment += c;
                i       += 2;
                continue;
            }
            isvalid = false;
            return;
        }
        else
            segment += path[i];
    }
    if (!segment.empty())
        segments.push_back(segment);

    path.clear();
    std::list<std::string>::iterator it = segments.begin();
    while (it != segments.end())
    {
        path += *it;
        it++;
    }
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
