#include "ConfigParser.hpp"
#include <cstdlib>

// static members
const std::string ConfigParser::serverDirective[] = { "listen", "server_name",
                                        "root", "index", "client_max_body_size",
                                        "error_page" };

const ConfigParser::serverHandlers ConfigParser::serverEntry[] = {
            &ConfigParser::handleListen,
            &ConfigParser::handleServerName,
            &ConfigParser::handleRoot,
            &ConfigParser::handleIndex,
            &ConfigParser::handleClientMaxBody,
            &ConfigParser::handleErrorPage
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


void ConfigParser::handleListen(ServerConfig& server) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("listen: expected address or port");

    const std::string value = _tokens[_i].value;
    ListenConfig listen;

    std::size_t colonPos = value.find(':');

    // case 1: only port -> listen 8080;
    if (colonPos == std::string::npos) {
        for (std::size_t j = 0; j < value.size(); ++j) {
            if (!std::isdigit(value[j]))
                throwError("listen: port must be numeric");
        }
        if (value.empty())
            throwError("listen: empty port");

        std::size_t port = std::strtoul(value.c_str(), NULL, 10);
        if (port > 65535)
            throwError("listen: port out of range");

        listen.host = "0.0.0.0";
        listen.port = static_cast<int>(port);
    }
    // case 2: host:port -> listen 127.0.0.1:8080;
    else {
        if (value.find(':', colonPos + 1) != std::string::npos)
            throwError("listen: invalid format");

        const std::string host = value.substr(0, colonPos);
        const std::string portStr = value.substr(colonPos + 1);

        if (host.empty())
            throwError("listen: host must not be empty");
        if (portStr.empty())
            throwError("listen: port must not be empty");

        if (!isValidIPv4(host) && !isValidHostname(host))
            throwError("listen: invalid host");

        for (std::size_t j = 0; j < portStr.size(); ++j) {
            if (!std::isdigit(portStr[j]))
                throwError("listen: port must be numeric");
        }

        std::size_t port = std::strtoul(portStr.c_str(), NULL, 10);
        if (port > 65535)
            throwError("listen: port out of range");

        listen.host = host;
        listen.port = static_cast<int>(port);
    }

    advance();
    expect(TOK_SEMICOLON, "listen: expected ';' after " + value, true);

    server.listens.push_back(listen);
}

// void ConfigParser::handleListenPort(ServerConfig& server) {
//     advance();
//     if (_tokens[_i].type != TOK_WORD)
//         throwError("listen_port: expected 'port'");
//
//     for (std::size_t i = 0; i < _tokens[_i].value.size(); i++)
//         if (!std::isdigit(_tokens[_i].value[i]))
//             throwError("listen_port: port must be numeric");
//
//     std::size_t port = std::strtoul(_tokens[_i].value.c_str(), NULL, 10);
//     if (port > 65535)
//         throwError("listen_port: port out of rang");
//     advance();
//     expect(TOK_SEMICOLON, "listen_port: expected ';' after " + _tokens[_i].value, true);
//     server.listen_port = port;
// }
//
// void ConfigParser::handleListenHost(ServerConfig& server) {
//     advance();
//     if (_tokens[_i].type != TOK_WORD)
//         throwError("listen_host: expected 'host'");
//
//     const std::string host = _tokens[_i].value;
//     if (host.empty())
//         throwError("listen_host: host must not be empty");
//
//     if (!isValidHostname(host) && !isValidIPv4(host))
//         throwError("listen_host: invalid host");
//     advance();
//     expect(TOK_SEMICOLON, "listen_host: expected ';' after " + host, true);
//     server.listen_host = host;
// }

void ConfigParser::handleServerName(ServerConfig& server) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("server_name: expected at least one hostname");
    while (_tokens[_i].type == TOK_WORD) {
        if (!isValidHostname(_tokens[_i].value))
            throwError("server_name: invalid hostname '" + _tokens[_i].value + "'");
        server.server_names.push_back(_tokens[_i].value);
        advance();
    }
    expect(TOK_SEMICOLON, "server_name: expected ';' after " + server.server_names.back(), true);
}

void ConfigParser::handleRoot(ServerConfig& server) {
    advance();

    if (_tokens[_i].type != TOK_WORD)
        throwError("root: expected a path");

    if (_tokens[_i].value[0] != '/')
        throwError("root: path must be absolute (start with '/')");

    server.root = _tokens[_i].value;
    advance();
    expect(TOK_SEMICOLON, "root: expected ';' after " + server.root, true);
}

void ConfigParser::handleIndex(ServerConfig& server) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("index: expected at least one index file");
    while (_tokens[_i].type == TOK_WORD) {
        server.index.push_back(_tokens[_i].value);
        advance();
    }
    expect(TOK_SEMICOLON, "index: expected ';' after " + server.index.back(), true);
    // advance();
}

void ConfigParser::handleClientMaxBody(ServerConfig& server) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("client_max_body_size: expected a size value");

    std::string raw = _tokens[_i].value;
    std::size_t bytes = parseSize(raw);

    server.client_max_body_size = bytes;

    advance();
    expect(TOK_SEMICOLON, "client_max_body_size: expected ';' after " + raw, true);
}


void ConfigParser::handleErrorPage(ServerConfig& server) {
    
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
            if (code < 100 || code > 599)
                throwError("error_pages: invalid code");
            codes.push_back(code);
            advance();
            continue ;
        }
        if (isValidPath(_tokens[_i].value)) {
            path = _tokens[_i].value;
            advance();
            break ;
        }
        else
            throwError("error_pages: expected <codes> and <path>");
    }
    if (codes.size() < 1)
        throwError("error_pages: expected <code>");
    expect(TOK_SEMICOLON, "error_pages: expected ';' after " + path, true);
    for (std::size_t i = 0; i < codes.size(); i++)
        server.error_pages[codes[i]] = path;
}
