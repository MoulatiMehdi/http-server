#ifndef HTTP_PARSER_BODY_HPP
#define HTTP_PARSER_BODY_HPP

#include "Buffer.hpp"
#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include <cstddef>

class HttpParserBody : virtual public HttpParserState
{
  protected:
    enum ChunkState
    {
        SW_CHUNK_START = 0,
        SW_CHUNK_SIZE,
        SW_CHUNK_SIZE_ALMOST_DONE,
        SW_CHUNK_DATA,
        SW_AFTER_DATA,
        SW_AFTER_DATA_ALMOST_DONE,
        SW_LAST_CHUNK_SIZE_ALMOST_DONE,
        SW_LAST_CHUNK_DATA_ALMOST_DONE,
        SW_BODY_ALMOST_DONE,
    };

    HttpParserBody();
    ~HttpParserBody();

    size_t m_chunk_max_size;
    size_t m_chunk_size;

    void clear();
    void parse_body(HttpRequest &request, Buffer &buffer);
    void parse_body_chunk(HttpRequest &request, Buffer &buffer);
    void parse_body_length(HttpRequest &request, Buffer &buffer);
    void parse_body_chunked(HttpRequest &request, const char *str, size_t len);
};
#endif
