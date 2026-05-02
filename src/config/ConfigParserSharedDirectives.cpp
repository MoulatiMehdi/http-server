#include "ConfigParser.hpp"

std::string ConfigParser::parseRootValue() {
    advance();

    if (_tokens[_i].type != TOK_WORD)
        throwError("root: expected a path");

    const std::string path = _tokens[_i].value;

    if (!isValidPath(path))
        throwError("root: path must be absolute (start and end with '/')");

    advance();
    expect(TOK_SEMICOLON, "root: expected ';' after " + path, true);

    return path;
}

std::vector<std::string> ConfigParser::parseIndexValues() {
    std::vector<std::string> result;

    advance();

    if (_tokens[_i].type != TOK_WORD)
        throwError("index: expected at least one index file");

    while (_tokens[_i].type == TOK_WORD) {
        result.push_back(_tokens[_i].value);
        advance();
    }
    expect(TOK_SEMICOLON, "index: expected ';' after " + result.back(), true);

    return result;
}

std::size_t ConfigParser::parseClientMaxBodyValue() {
    advance();

    if (_tokens[_i].type != TOK_WORD)
        throwError("client_max_body_size: expected a value");

    std::string raw = _tokens[_i].value;
    std::size_t bytes = parseSize(raw);

    advance();
    expect(TOK_SEMICOLON,
        "client_max_body_size: expected ';' after " + raw, true);

    return bytes;
}

