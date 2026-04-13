#include "HttpParserState.hpp"
#include "HttpRequest.hpp"
#include "ParserError.hpp"
#include <iostream>
#include <string>

const char *ascii_repr[128] = {
    "\\0",
    "\\x01",
    "\\x02",
    "\\x03",
    "\\x04",
    "\\x05",
    "\\x06",
    "\\a",
    "\\b",
    "\\t",
    "\\n",
    "\\v",
    "\\f",
    "\\r",
    "\\x0E",
    "\\x0F",
    "\\x10",
    "\\x11",
    "\\x12",
    "\\x13",
    "\\x14",
    "\\x15",
    "\\x16",
    "\\x17",
    "\\x18",
    "\\x19",
    "\\x1A",
    "\\x1B",
    "\\x1C",
    "\\x1D",
    "\\x1E",
    "\\x1F",
    " ",
    "!",
    "\"",
    "#",
    "$",
    "%",
    "&",
    "'",
    "(",
    ")",
    "*",
    "+",
    ",",
    "-",
    ".",
    "/",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ":",
    ";",
    "<",
    "=",
    ">",
    "?",
    "@",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "[",
    "\\\\",
    "]",
    "^",
    "_",
    "`",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "s",
    "t",
    "u",
    "v",
    "w",
    "x",
    "y",
    "z",
    "{",
    "|",
    "}",
    "~",
    "\\x7F"
};

HttpParserState::HttpParserState()
    : m_error(ParserError::ok),
      m_state(0),
      m_complete(false),
      m_chunked(false),
      m_discard_body(false),
      m_phase(P_REQUEST_LINE),
      m_cache()
{
}

HttpParserState::HttpParserState(HttpParserState &)
{
}

HttpParserState &HttpParserState::operator=(HttpParserState &)
{
    return *this;
}

void HttpParserState::processError(HttpRequest &request)
{
    switch (m_error)
    {
        case ParserError::ok:
            return request.setStatus(Status::OK);
        case ParserError::bad_request:
        case ParserError::bad_line_ending:
        case ParserError::bad_target:
        case ParserError::bad_version:
        case ParserError::bad_method:
        case ParserError::bad_status:
        case ParserError::bad_reason:
        case ParserError::bad_field:
        case ParserError::bad_header_name:
        case ParserError::bad_header_value:
        case ParserError::bad_content_length:
        case ParserError::multiple_content_length:
        case ParserError::bad_value:
            return request.setStatus(Status::BAD_REQUEST);
        case ParserError::unsupported_version:
            return request.setStatus(Status::HTTP_VERSION_NOT_SUPPORTED);
        case ParserError::unsupported_method:
            return request.setStatus(Status::METHOD_NOT_ALLOWED);
        case ParserError::unsupported_transfer:
            return request.setStatus(Status::NOT_IMPLEMENTED);
        case ParserError::header_field_name_too_large:
        case ParserError::header_field_value_too_large:
            return request.setStatus(Status::REQUEST_HEADER_FIELDS_TOO_LARGE);
        case ParserError::stale_parser:
        case ParserError::short_read:
            break;
    }
}

void HttpParserState::setError(ParserError err)
{
    if (m_error == ParserError::ok)
        m_error = err;
}

bool HttpParserState::good() const
{
    return m_error == ParserError::ok;
}

void HttpParserState::clear()
{
    m_state        = 0;
    m_phase        = P_REQUEST_LINE;
    m_complete     = false;
    m_error        = ParserError::ok;
    m_discard_body = false;
    m_chunked      = false;
}

std::ostream &operator<<(std::ostream &os, const HttpParserState &hps)
{
    std::string phase[3] = {"Request Line", "Headers", "Body"};

    std::cout << "Error      : " << to_string(hps.m_error) << std::endl;
    if (!hps.m_complete)
        std::cout << "Phase      : " << phase[hps.m_phase] << std::endl;
    else
        std::cout << "Phase      : Complete" << std::endl;
    if (!hps.good())
        std::cout << "State      : " << hps.m_state << std::endl;
    std::cout << "Buffer     : '";

    for (int i = 0; i < hps.m_cache.size(); i++)
    {
        std::cout << ascii_repr[hps.m_cache[i]];
    }
    std::cout << "'" << std::endl;

    return os;
}

HttpParserState::~HttpParserState()
{
}
