#include "ConfigParser.hpp"
#include <cstdlib>
#include "Method.hpp"
// static

const std::string ConfigParser::locationDirective[] = {
    "allowed_methods",
    "return",
    "root",
    "index",
    "autoindex",
    "upload_dir",
    "client_max_body_size",
    "cgi"
};

const ConfigParser::locationHandlers ConfigParser::locationEntry[] = {
    &ConfigParser::handleLocAllowMethods,
    &ConfigParser::handleLocReturn, // !
    &ConfigParser::handleLocRoot,
    &ConfigParser::handleLocIndex,
    &ConfigParser::handleLocAutoindex,
    &ConfigParser::handleLocUploadDir,
    &ConfigParser::handleLocClientMaxBody,
    &ConfigParser::handleLocCgi
};

const std::size_t ConfigParser::locationDirCount =
    sizeof(ConfigParser::locationEntry) / sizeof(ConfigParser::locationEntry[0]);

// -----

LocationConfig ConfigParser::parseLocationBlock() {
    advance();
    expect(TOK_WORD, "expected location path", false);
    // const std::string path = _tokens[_i].value;
    // std::cout << path << std::endl;
    // while(true) ;
    if (isValidPath(_tokens[_i].value) == false
        && _tokens[_i].value[_tokens[_i].value.size() - 1] == '/') // update it later
        throwError("location: path must start and end with '/'");
    LocationConfig location;
    location.path = _tokens[_i].value;
    advance();
    
    expect(TOK_LBRACE, "expected '{' after location's path", true);
    while(_tokens[_i].type != TOK_RBRACE) {
        if (_tokens[_i].type == TOK_EOF)
            throw std::runtime_error("unexpected EOF inside server block");
        parseLocationDirective(location);
    }
    expect(TOK_RBRACE, "expected '}' after 'location' block", true);
    return location;
}

void ConfigParser::parseLocationDirective(LocationConfig& location) { // Template!
    for (std::size_t i = 0; i < locationDirCount; i++) {
        if (_tokens[_i].value == locationDirective[i]) {
            (this->*locationEntry[i])(location);
            return ;
        }
    }
    throwError("unknown location directive '" + _tokens[_i].value + "'");
}

void ConfigParser::handleLocAllowMethods(LocationConfig& loc) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("allowed_methods: expected at least one method");

    while (_tokens[_i].type == TOK_WORD) {
        Method method = string_to_method(_tokens[_i].value);
        if (method == method::UNKNOWN)
            throwError("allowed_methods: only GET/POST/DELETE allowed");
        loc.allowed_methods.push_back(method);
        advance();
    }
    expect(TOK_SEMICOLON, "allowed_methods: expected ';' after " + _tokens[_i].value, true);
}

// return <code> <url>
void ConfigParser::handleLocReturn(LocationConfig& loc) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("return: expected status code");
    const std::string codeStr = _tokens[_i].value; // there a function that check code in serverv funcs
    for (std::size_t i = 0; i < codeStr.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(codeStr[i])))
            throwError("return: code must be numeric");
    int code = std::atoi(codeStr.c_str());
    if (code < 300 || code > 399)
        throwError("return: code must be 3xx");
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("return: expected redirect url");
    loc.redirect_code = code;
    loc.redirect_url = _tokens[_i].value;
    advance();
    expect(TOK_SEMICOLON, "return: expected ';' after " + loc.redirect_url, true);
}

void ConfigParser::handleLocRoot(LocationConfig& loc) {
    loc.root = parseRootValue();
}

void ConfigParser::handleLocIndex(LocationConfig& loc) {
    loc.index = parseIndexValues();
}

// autoindex on|off
void ConfigParser::handleLocAutoindex(LocationConfig& loc) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("autoindex: expected on|off");
    const std::string val = _tokens[_i].value;
    if (val == "on") loc.autoindex = true;
    else if (val == "off") loc.autoindex = false;
    else throwError("autoindex: must be on or off");
    advance();
    expect(TOK_SEMICOLON, "autoindex: expected ';' after " + val, true);
}

// upload_dir <path>
void ConfigParser::handleLocUploadDir(LocationConfig& loc) {
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("upload_dir: expected path");
    const std::string path = _tokens[_i].value;
    if (!isValidPath(path))
        throwError("upload_dir: invalid path");
    loc.upload_dir = path;
    advance();
    expect(TOK_SEMICOLON, "upload_dir: expected ';' after " + loc.upload_dir, true);
}

// client_max_body_size <size>[k|m|g]
void ConfigParser::handleLocClientMaxBody(LocationConfig& loc) { // copied from server
    loc.client_max_body_size = parseClientMaxBodyValue();
}

void ConfigParser::handleLocCgi(LocationConfig& loc) { // cgi .py /usr/bin/python3;
    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("cgi: expected extension");
    std::string ext = _tokens[_i].value;
    if (ext[0] != '.') // should I check if it exactly .ph || .php ?
        throwError("cgi: extension must start with '.'");

    advance();
    if (_tokens[_i].type != TOK_WORD)
        throwError("cgi: expected executable path");
    std::string exec = _tokens[_i].value;
    if (!isValidFilePath(exec))
        throwError("cgi: invalid executable path");

    loc.cgi[ext] = exec;

    advance();
    expect(TOK_SEMICOLON, "cgi: expected ';'", true);
}
