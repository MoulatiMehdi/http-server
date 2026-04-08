#include "ConfigParser.hpp"
#include "Tokenizer.hpp"

std::string ConfigParser::readFileOrThrow(const std::string& path) {
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
    while (_tokens[_i].type != TOK_EOF) {
        cfg.servers.push_back(parseServerBlock()); // try catch!
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

