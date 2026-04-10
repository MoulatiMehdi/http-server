#include "ConfigParser.hpp"


void ConfigParser::throwError(const std::string& msg) {
    throw std::runtime_error(msg + " at line " + toString(_tokens[_i].line) +
                             ", col " + toString(_tokens[_i].col));
}

void ConfigParser::advance() {
    if (_tokens[_i + 1].type != TOK_EOF)
        _i++;
}

void ConfigParser::expect(const char c, const std::string& msg, bool adv) {
    if (_tokens[_i].type != TOK_WORD && _tokens[_i].value[0] != c)
        throwError(msg);
    if (adv)
        advance();
}

void ConfigParser::expect(const std::string& word, const std::string& msg, bool adv) {
    if (_tokens[_i].type != TOK_WORD && _tokens[_i].value != word)
        throwError(msg);
    if (adv)
        advance();
}

void ConfigParser::expect(const TokenType type, const std::string& msg, bool adv) {
    if (_tokens[_i].type != type)
        throwError(msg);
    if (adv)
        advance();
}

std::string ConfigParser::toString(std::size_t n) {
    std::ostringstream ss;
    ss << n;
    return ss.str();
}
