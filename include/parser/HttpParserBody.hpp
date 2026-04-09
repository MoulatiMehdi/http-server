#ifndef HTTP_PARSER_BODY_HPP
#define HTTP_PARSER_BODY_HPP
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <cstddef>
#include <string>

class HttpParserBody : virtual public HttpParserState
{
  protected:

    enum ChunkState
    {
        sw_chunk_start = 0,
        sw_chunk_size,
        sw_chunk_size_almost_done,
        sw_chunk_data,
        sw_after_data,
        sw_after_data_almost_done,
        sw_last_chunk_size_almost_done,
        sw_last_chunk_data_almost_done,
        sw_body_almost_done,
    };

    HttpParserBody();
    ~HttpParserBody();

    std::string m_buff;
    size_t      m_body_size;
    size_t      m_chunk_max_size;
    size_t      m_chunk_size;
    std::string m_chunk_value;
    ChunkState  m_chunk_state;

    void   parse_body(HttpRequest &request, const char *str, size_t len);
    size_t parse_body_chunk(HttpRequest &request, const char *str, size_t len);
    size_t parse_body_length(HttpRequest &request, const char *str, size_t len);
    HttpParserState::Action
    parse_body_chunked(HttpRequest &request, const char *str, size_t len);
};
#endif
