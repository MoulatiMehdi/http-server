

#include "Buffer.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "ParserError.hpp"
#include "Status.hpp"
#include <cerrno>
#include <iostream>

enum UploadState
{
    SW_UPLOAD_BOUNDARY_START = 0,
    SW_UPLOAD_BOUNDARY_CHECK,
    SW_UPLOAD_BOUNDARY_ALMOST_DONE,
    SW_UPLOAD_HEADERS,
    SW_UPLOAD_FILE_DATA,
    SW_UPLOAD_FILE_BOUNDARY,
    SW_UPLOAD_FILE_ALMOST_DONE,
    SW_UPLOAD_BODY_ALMOST_DONE,
};

#define CR '\r'
#define LF '\n'

void HttpRequestParser::parse_upload_body(Buffer &buffer)
{
    std::cerr << "parse_upload_body" << std::endl;
    if (m_content_type == "multipart/form-data")
        parse_multipart(buffer);
    else
        parse_body_by_length(buffer);
}

void HttpRequestParser::parse_multipart(Buffer &buffer)
{
    while (!buffer.empty())
    {
        char ch;
        if (m_state != SW_UPLOAD_HEADERS)
            ch = buffer.getc();
        switch (m_state)
        {
            case SW_UPLOAD_BOUNDARY_START:
                if (m_boundary.size() > m_index)
                {
                    if (m_boundary[m_index] != ch)
                    {
                        return setError(error::bad_request);
                    }

                    m_index++;
                    break;
                }
                switch (ch)
                {
                    case CR:
                        m_state = SW_UPLOAD_BOUNDARY_ALMOST_DONE;
                        break;
                    default:
                        return setError(error::bad_request);
                }
                break;
            case SW_UPLOAD_BOUNDARY_ALMOST_DONE:
                switch (ch)
                {
                    case LF:
                        m_state = SW_UPLOAD_HEADERS;
                        m_response.clear();
                        break;
                    default:
                        return setError(error::bad_request);
                }
                m_state = SW_UPLOAD_HEADERS;
                break;
            case SW_UPLOAD_HEADERS:

                m_response.parser().parse_headers(buffer);
                if (!m_response.good())
                {
                    return setError(error::bad_request);
                }
                if (m_response.complete())
                {
                    m_filename = m_response.extract_key(
                        "Content-Disposition", "filename"
                    );
                    if (m_filename.empty())
                    {
                        return setError(error::bad_request);
                    }
                    m_filename = route.path + m_filename;

                    if (m_response.body().open_file(m_filename, false) < 0)
                    {
                        Logger::error("SW_UPLOAD_HEADERS");
                        return setError(error::bad_upload);
                    }
                    else
                        Logger::info(m_filename + " created");

                    m_state = SW_UPLOAD_FILE_DATA;
                }
                break;
            case SW_UPLOAD_FILE_DATA:
                if (ch == CR)
                {
                    m_index = 0;
                    m_state = SW_UPLOAD_FILE_ALMOST_DONE;
                    break;
                }
                m_response.body().append(ch);

                break;
            case SW_UPLOAD_FILE_ALMOST_DONE:
                if (ch == LF)
                {
                    m_index = 0;
                    m_state = SW_UPLOAD_FILE_BOUNDARY;
                }
                else
                {
                    m_response.body().append("\r");
                    if (ch == CR)
                        m_state = SW_UPLOAD_FILE_ALMOST_DONE;
                    else
                    {
                        m_response.body().append(ch);
                        m_state = SW_UPLOAD_FILE_DATA;
                    }
                }
                break;

            case SW_UPLOAD_FILE_BOUNDARY:
                if (m_boundary.size() <= m_index)
                {
                    switch (ch)
                    {
                        case CR:
                            m_state = SW_UPLOAD_BOUNDARY_ALMOST_DONE;
                            break;
                        case '-':
                            m_index = 1;
                            m_state = SW_UPLOAD_BODY_ALMOST_DONE;
                            break;
                        default:
                            return setError(error::bad_request);
                    }
                    break;
                }
                if (m_boundary[m_index] != ch)
                {
                    m_response.body().append("\r\n", 2);
                    m_response.body().append(m_boundary.c_str(), m_index);
                    switch (ch)
                    {
                        case CR:
                            m_state = SW_UPLOAD_FILE_ALMOST_DONE;
                            break;
                        default:
                            m_response.body().append(ch);
                            m_state = SW_UPLOAD_FILE_DATA;
                    }
                    break;
                }
                m_index++;
                break;
            case SW_UPLOAD_BODY_ALMOST_DONE:
                switch (ch)
                {
                    case '-':
                        Logger::info("Upload finished successfully");
                        m_request.setComplete(true);
                        m_request.setStatus(status::CREATED);
                        m_response.body().close();
                        m_state = 0;
                        return;
                        break;
                    default:
                        return setError(error::bad_request);
                }
                break;
        }
    }
}
