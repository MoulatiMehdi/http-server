#include "ConfigParser.hpp"
#include "Config.hpp"
#include "Tokenizer.hpp"
#include "sys/stat.h"
#include <stdexcept>
#include <sys/socket.h>


bool isRegularFile(const std::string &path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
        return true;
    }
    return false;
}

std::string ConfigParser::readFileOrThrow(const std::string& path) {
	if  (!isRegularFile(path))
        throw std::runtime_error("Error: could not open config file " + path);
    std::ifstream file(path.c_str());
    if (!file.is_open())
        throw std::runtime_error("Error: could not open config file " + path);

    std::ostringstream ss;
    ss << file.rdbuf();

    if (file.bad()) // !
        throw std::runtime_error("Error while reading config file " + path);

    return ss.str();
}

Config ConfigParser::parseTokens(const std::vector<Token>& tokens) {
    _tokens = tokens;
    _i = 0;
    Config cfg;
    ServerConfig server;

    while (_tokens[_i].type != TOK_EOF) {
        server = parseServerBlock();
        checkMandatoryServerDirectives(server);
        cfg.servers.push_back(server); // try catch!
        // check mandatory directives!

    }
    return cfg;
}

Config ConfigParser::parseFile(const std::string& path) {
    try {
        std::string ss = readFileOrThrow(path);
        Tokenizer   tz(ss);
        std::vector<Token> tokens = tz.tokenize();
        return parseTokens(tokens);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl; // rethrow!? cerr instead of cout !
        throw; // how?
    }
}

