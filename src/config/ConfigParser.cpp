#include "ConfigParser.hpp"
#include "Config.hpp"
#include "Tokenizer.hpp"
#include "sys/stat.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>


static bool isRegularFile(const std::string &path) {
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

    if (file.bad())
        throw std::runtime_error("Error while reading config file " + path);

    return ss.str();
}

void ConfigParser::inheritServerRootToLocations(ServerConfig& server) {
    for (std::size_t i = 0; i < server.locations.size(); i++) {
        if (server.locations[i].root.empty()) 
            server.locations[i].root = server.root + server.locations[i].path;
    }
}

Config ConfigParser::parseTokens(const std::vector<Token>& tokens) {
    _tokens = tokens;
    _i = 0;
    Config cfg;
    ServerConfig server;

    while (_tokens[_i].type != TOK_EOF) {
        server = parseServerBlock();
        checkMandatoryServerDirectives(server);
        inheritServerRootToLocations(server);
        cfg.servers.push_back(server);
    }
    return cfg;
}

Config ConfigParser::setDefaultConfig() {
    Config          config;
    ServerConfig    server;
    LocationConfig  location;

    location.path = "/";
    location.root = ".";
    location.autoindex = true;

    server.locations.push_back(location);
    server.listens.push_back(ListenConfig());
    server.root = ".";

    config.servers.push_back(server);

    return config;
}

Config ConfigParser::parseFile(const std::string& path)
{
    if (path.empty())
        return setDefaultConfig();

    std::string ss = readFileOrThrow(path);
    Tokenizer   tz(ss);
    std::vector<Token> tokens = tz.tokenize();
    return parseTokens(tokens);
} 
