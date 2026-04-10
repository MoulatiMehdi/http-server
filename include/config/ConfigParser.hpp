#ifndef   CONFIGPARSER_HPP
# define  CONFIGPARSER_HPP

# include <vector>
# include <string>
# include <fstream>
# include <sstream>
# include <iostream>
# include "Tokenizer.hpp"  // Token, TokenType
# include "Config.hpp"

class ConfigParser {
private:
    std::vector<Token>  _tokens;
    std::size_t         _i;

    typedef void (ConfigParser::*serverHandlers)(ServerConfig& server); //check it again
    typedef void (ConfigParser::*locationHandlers)(LocationConfig& loc); //check it again

    static const std::string    serverDirective[];
    static const serverHandlers serverEntry[];
    static const std::size_t    serverDirCount;

    static const std::string    locationDirective[];
    static const locationHandlers locationEntry[];
    static const std::size_t    locationDirCount;
    
    // Utils --
    void throwError(const std::string& msg);
    void advance();
    void expect(const std::string& word, const std::string& msg, bool advance);
    void expect(const TokenType type, const std::string& msg, bool advance);
    void expect(const char c, const std::string& msg, bool advance);
    std::string toString(std::size_t n);

    // Validators --
    bool isValidHostname(const std::string& host);
    bool isValidIPv4(const std::string& ip);
    bool isAllDigit(const std::string& num);
    bool isValidPath(const std::string& path);
    std::size_t parseSize(const std::string& raw);

    std::string     readFileOrThrow(const std::string& path);
    Config          parseTokens(const std::vector<Token>& tokens);
    ServerConfig    parseServerBlock();
    LocationConfig  parseLocationBlock();
    void            parseLocationDirective(LocationConfig& location);
    void            parseServerDirective(ServerConfig& server);

    // parseServerHandlers
    void handleListen(ServerConfig& server);
    void handleServerName(ServerConfig& server);
    void handleRoot(ServerConfig& server);
    void handleIndex(ServerConfig& server);
    void handleClientMaxBody(ServerConfig& server);
    void handleErrorPage(ServerConfig& server);

    // parseLocationDirective
    void handleLocAllowMethods(LocationConfig& loc);
    void handleLocReturn(LocationConfig& loc);
    void handleLocRoot(LocationConfig& loc);
    void handleLocIndex(LocationConfig& loc);
    void handleLocAutoindex(LocationConfig& loc);
    void handleLocUploadDir(LocationConfig& loc);
    void handleLocClientMaxBody(LocationConfig& loc);
    void handleLocCgi(LocationConfig& loc);

public:
    Config parseFile(const std::string& path);
};

#endif
