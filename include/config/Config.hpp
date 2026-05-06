#ifndef   CONFIG_HPP
# define  CONFIG_HPP

# include <string>
# include <vector>
# include <map>
# include "Method.hpp"
# include "MimeType.hpp"

struct ListenConfig {
    std::string host;                   // ip
    int port;                           // 8080

    ListenConfig() : host("0.0.0.0"), port(80) {}
};

typedef std::vector<std::string> IndexTable;

struct LocationConfig {
    std::string path;                          // "/" TODO: already mandatory
    std::vector<Method> allowed_methods;       // GET POST DELETE TODO: at least one!

    std::string root;                          // optional override
    std::vector<std::string> index;            // e.g. index.html
    bool autoindex;                            // true/false

    std::map<std::string, std::string> cgi;    // e.g. cgi[".py"] = "/usr/bin/python3";

    std::size_t client_max_body_size;
    int redirect_code;                         // 0 => none, else 301/302... return
    std::string redirect_url;                  // redirect target            return

    LocationConfig()
        : autoindex(false),
          client_max_body_size(0),
          redirect_code(0) {}
};

struct ServerConfig {
    static MimeType mimetype;
    std::vector<ListenConfig> listens;         // multiple interface:port pairs TODO: 1..*
    std::vector<std::string> server_names;     // a.com www.a.com TODO: delete it
    std::string root;                          // /var/www/a TODO: mandatory
    std::vector<std::string> index;            // index.html 
    std::size_t client_max_body_size;          // bytes 
    std::map<int, std::string> error_pages;    // 404 -> /errors/404.html
    std::vector<LocationConfig> locations;     // Good

    std::string errorPage(int code) const;

    ServerConfig() : client_max_body_size(1024 * 1024) {}
};

struct Config {
    std::vector<ServerConfig> servers;
};

#endif
