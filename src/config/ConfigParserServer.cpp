#include "ConfigParser.hpp"
#include <cstdlib>

// static members
const std::string ConfigParser::serverDirective[] = { "listen", "server_name",
                                        "root", "index", "client_max_body_size",
                                        "error_page" };

const ConfigParser::serverHandlers ConfigParser::serverEntry[] = {
            &ConfigParser::handleServListen,
            &ConfigParser::handleServServerName,
            &ConfigParser::handleServRoot,
            &ConfigParser::handleServIndex,
            &ConfigParser::handleServClientMaxBody,
            &ConfigParser::handleServErrorPage
};

const std::size_t ConfigParser::serverDirCount = 
           sizeof(ConfigParser::serverEntry) / sizeof(ConfigParser::serverEntry[0]);

// --------

ServerConfig ConfigParser::parseServerBlock() {
    expect("server", "expected 'server' at top-level", true);
    expect(TOK_LBRACE, "expected '{' after 'server'", true);

    ServerConfig server;

    while(_tokens[_i].type != TOK_RBRACE) {
        if (_tokens[_i].type == TOK_EOF)
            throw std::runtime_error("unexpected EOF inside server block");
        if (_tokens[_i].value == "location") {
            server.locations.push_back(parseLocationBlock());
            continue ;
        }
        parseServerDirective(server);
    }
    expect(TOK_RBRACE, "expected '}' at the end of 'server' block", true);
    if (_tokens[_i + 1].type == TOK_EOF)
        _i++;
    return server;
}

void ConfigParser::parseServerDirective(ServerConfig& server) { 
    for (std::size_t i = 0; i < serverDirCount; i++) {
        if (_tokens[_i].value == serverDirective[i]) {
            (this->*serverEntry[i])(server);
            return ;
        }
    }
    throwError("unknown server directive '" + _tokens[_i].value + "'");
}
