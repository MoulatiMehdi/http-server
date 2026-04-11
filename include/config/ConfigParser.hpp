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

    // -- Read from file --
    std::string     readFileOrThrow(const std::string& path);
    
    // -- Parse Tokens --
    Config          parseTokens(const std::vector<Token>& tokens);

    // -- parseFile's big functions --
    ServerConfig    parseServerBlock();
    LocationConfig  parseLocationBlock();
    void            parseLocationDirective(LocationConfig& location);
    void            parseServerDirective(ServerConfig& server);

    // -- Server directives --
    void handleServListen(ServerConfig& server);
    void handleServServerName(ServerConfig& server);
    void handleServRoot(ServerConfig& server);
    void handleServIndex(ServerConfig& server);
    void handleServClientMaxBody(ServerConfig& server);
    void handleServErrorPage(ServerConfig& server);

    // -- Location directives --
    void handleLocAllowMethods(LocationConfig& loc);
    void handleLocReturn(LocationConfig& loc);
    void handleLocRoot(LocationConfig& loc);
    void handleLocIndex(LocationConfig& loc);
    void handleLocAutoindex(LocationConfig& loc);
    void handleLocUploadDir(LocationConfig& loc);
    void handleLocClientMaxBody(LocationConfig& loc);
    void handleLocCgi(LocationConfig& loc);

    // -- Shared Directives --
    std::string parseRootValue();
    std::size_t parseClientMaxBodyValue();
    std::vector<std::string> parseIndexValues();

    // Listen's functions
    std::string   parseListenValueToken();
    ListenConfig  parseListenPortOnly(const std::string& value);
    ListenConfig  parseListenHostPort(const std::string& value, std::size_t colonPos);
    int           parseListenPort(const std::string& value);


    // -- Utils --
    void throwError(const std::string& msg);
    void advance();
    void expect(const std::string& word, const std::string& msg, bool advance);
    void expect(const TokenType type, const std::string& msg, bool advance);
    void expect(const char c, const std::string& msg, bool advance);
    std::string toString(std::size_t n);

    // -- Validators --
    bool isValidHostname(const std::string& host);
    bool isValidIPv4(const std::string& ip);
    bool isAllDigit(const std::string& num);
    bool isValidPath(const std::string& path);
    std::size_t parseSize(const std::string& raw);

public:
    // -- Parent function --
    Config parseFile(const std::string& path);

    // -- Debugging --
    void  printConfig(const Config& config);
};

#endif
