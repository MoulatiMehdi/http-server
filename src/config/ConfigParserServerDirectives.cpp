#include "ConfigParser.hpp"
#include "Tokenizer.hpp"
#include <cstdlib>

void ConfigParser::handleServListen(ServerConfig& server) {
    const std::string value = parseListenValueToken();
    ListenConfig listen;

    std::size_t colonPos = value.find(':');

    if (colonPos == std::string::npos)
        listen = parseListenPortOnly(value);
    else
        listen = parseListenHostPort(value, colonPos);

    advance();
    expect(TOK_SEMICOLON, "listen: expected ';' after " + value, true);

    server.listens.push_back(listen);
}

void ConfigParser::handleServRoot(ServerConfig& server) {
    server.root = parseRootValue();
}

void ConfigParser::handleServIndex(ServerConfig& server) {
    server.index = parseIndexValues();
}

void ConfigParser::handleServClientMaxBody(ServerConfig& server) {
    server.client_max_body_size = parseClientMaxBodyValue();
}


void ConfigParser::handleServErrorPage(ServerConfig& server) {
    
    std::vector<std::size_t> codes;
    std::string path;
    
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("error_pages: expected <code> and <path>");
    
    while (true) {
        if (_tokens[_i].type != TOK_WORD)
            throwError("error_pages: expected <code>");
        if (isAllDigit(_tokens[_i].value)) {
            std::size_t code = std::strtoul(_tokens[_i].value.c_str(), NULL, 10);
            if (code < 400 || code > 599)
                throwError("error_pages: invalid code");
            codes.push_back(code);
            advance();
            continue ;
        }
        // if (isValidPath(_tokens[_i].value)) {
        if (_tokens[_i].type != TOK_WORD)
            throwError("error_pages: expected <codes> and <path>");
        path = _tokens[_i].value;
        advance();
        break ;
        // }
        // else
        //     throwError("error_pages: expected <codes> and <path>");
    }
    if (codes.size() < 1)
        throwError("error_pages: expected <code>");
    expect(TOK_SEMICOLON, "error_pages: expected ';' after " + path, true);
    for (std::size_t i = 0; i < codes.size(); i++)
        server.error_pages[codes[i]] = path;
}
