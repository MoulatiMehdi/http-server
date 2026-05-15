#include "ConfigParser.hpp"
#include <sstream>
#include <stdexcept>


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

std::string ConfigParser::toString(std::size_t n) { // make it static!
    std::ostringstream ss;
    ss << n;
    return ss.str();
}

// for debug.... remove later

#include <iostream>
#include <string>
#include <vector>
#include <map>

template<typename T>
static void printStringVector(const std::vector<T>& v) {
    if (v.empty()) {
        std::cout << "(none)";
        return;
    }
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i)
            std::cout << ", ";
        std::cout << v[i];
    }
}
// static void printStringVector(const std::vector<std::string>& v) {
//     if (v.empty()) {
//         std::cout << "(none)";
//         return;
//     }
//     for (std::size_t i = 0; i < v.size(); ++i) {
//         if (i)
//             std::cout << ", ";
//         std::cout << v[i];
//     }
// }

static void printCgiMap(const std::map<std::string, std::string>& cgi) {
    if (cgi.empty()) {
        std::cout << "      CGI:                (none)\n";
        return;
    }

    std::cout << "      CGI:\n";
    for (std::map<std::string, std::string>::const_iterator it = cgi.begin();
         it != cgi.end(); ++it) {
        std::cout << "        - " << it->first << "  ->  " << it->second << "\n";
    }
}

void ConfigParser::printConfig(const Config& config) {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "                 PARSED CONFIGURATION DUMP                  \n";
    std::cout << "============================================================\n";

    if (config.servers.empty()) {
        std::cout << "No servers found.\n";
        std::cout << "============================================================\n";
        return;
    }

    for (std::size_t s = 0; s < config.servers.size(); ++s) {
        const ServerConfig& srv = config.servers[s];

        std::cout << "\n";
        std::cout << "------------------------------------------------------------\n";
        std::cout << "SERVER #" << s << "\n";
        std::cout << "------------------------------------------------------------\n";

        // Listen
        std::cout << "  LISTEN:\n";
        if (srv.listens.empty()) {
            std::cout << "    (none)\n";
        } else {
            for (std::size_t i = 0; i < srv.listens.size(); ++i) {
                std::cout << "    - " << srv.listens[i].host
                          << ":" << srv.listens[i].port << "\n";
            }
        }

        // Basic server info
        std::cout << "  ROOT:                  "
                  << (srv.root.empty() ? "(none)" : srv.root) << "\n";

        std::cout << "  INDEX:                 ";
        printStringVector(srv.index);
        std::cout << "\n";

        std::cout << "  SERVER NAMES:          ";
        printStringVector(srv.server_names);
        std::cout << "\n";

        std::cout << "  CLIENT MAX BODY SIZE:  "
                  << srv.client_max_body_size << "\n";

        // Error pages
        std::cout << "  ERROR PAGES:\n";
        if (srv.error_pages.empty()) {
            std::cout << "    (none)\n";
        } else {
            for (std::map<int, std::string>::const_iterator it = srv.error_pages.begin();
                 it != srv.error_pages.end(); ++it) {
                std::cout << "    - " << it->first << "  ->  " << it->second << "\n";
            }
        }

        // Locations
        std::cout << "  LOCATIONS:\n";
        if (srv.locations.empty()) {
            std::cout << "    (none)\n";
            continue;
        }

        for (std::size_t l = 0; l < srv.locations.size(); ++l) {
            const LocationConfig& loc = srv.locations[l];

            std::cout << "\n";
            std::cout << "    ........................................................\n";
            std::cout << "    LOCATION #" << l
                      << "   PATH: " << (loc.path.empty() ? "(empty)" : loc.path) << "\n";
            std::cout << "    ........................................................\n";

            std::cout << "      Allowed methods:    ";
            printStringVector(loc.allowed_methods);
            std::cout << "\n";

            std::cout << "      Root:               "
                      << (loc.root.empty() ? "(inherit/none)" : loc.root) << "\n";

            std::cout << "      Index:              ";
            printStringVector(loc.index);
            std::cout << "\n";

            std::cout << "      Autoindex:          "
                      << (loc.autoindex ? "on" : "off") << "\n";

            std::cout << "      Client max body:    ";
            if (loc.client_max_body_size == 0)
                std::cout << "(inherit)";
            else
                std::cout << loc.client_max_body_size;
            std::cout << "\n";

            std::cout << "      Redirect:           ";
            if (loc.redirect_code == 0)
                std::cout << "(none)";
            else
                std::cout << loc.redirect_code << " -> " << loc.redirect_url;
            std::cout << "\n";

            printCgiMap(loc.cgi);
        }
    }

    std::cout << "\n";
    std::cout << "============================================================\n";
}
