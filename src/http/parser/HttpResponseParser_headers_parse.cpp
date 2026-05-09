#include "HttpParserState.hpp"
#include "HttpResponse.hpp"
#include "HttpResponseParser.hpp"


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
            case RES_HDR_ERROR:
                return;
            case RES_HEADER_DONE:
                process_headers();
                m_response.setComplete(true);
                m_phase = PHASE_BODY;
                return;
            case RES_HEADER_LINE_DONE:
                process_header_line();
                break;
            case RES_HDR_CONTINUE:
                break;
        }
        if (!good())
            return;
    }
}
