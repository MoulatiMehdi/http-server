#include "HttpParserState.hpp"
#include "HttpResponseParser.hpp"
#include <cstddef>


void HttpResponseParser::parse_headers(Buffer &buff)
{
    const static Handler handlers[6] = {
        &HttpResponseParser::hdr_start,
        &HttpResponseParser::hdr_name,
        &HttpResponseParser::hdr_space_before_value,
        &HttpResponseParser::hdr_value,
        &HttpResponseParser::hdr_almost_done,
        &HttpResponseParser::hdr_header_almost_done,
    };
    while (!buff.empty())
    {
        char         ch     = buff.getc();
        unsigned int action = (this->*handlers[m_state])(ch);

        switch (action)
        {
            case RES_ERROR:
                return;
            case RES_HEADER_DONE:
                process_headers();
                request.setComplete(true);
                m_phase = PHASE_BODY;
                return;
            case RES_HEADER_LINE_DONE:
                process_header_line();
                break;
            case RES_CONTINUE:
                break;
        }
        if (!good())
            return;
    }
}
