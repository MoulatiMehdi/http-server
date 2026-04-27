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

struct LocationConfig {
    std::string path;                          // "/"
    std::vector<Method> allowed_methods;  // GET POST DELETE

    std::string root;                          // optional override
    std::vector<std::string> index;            // e.g. index.html
    bool autoindex;                            // true/false

    std::map<std::string, std::string> cgi;    // e.g. cgi[".py"] = "/usr/bin/python3";
    std::string upload_dir;                    // e.g. /tmp/uploads
    bool        upload_enabled;                // upload allowed or not

    std::size_t client_max_body_size;
    int redirect_code;                         // 0 => none, else 301/302... return
    std::string redirect_url;                  // redirect target            return

    LocationConfig()
        : autoindex(false),
          upload_enabled(false),
          client_max_body_size(0),
          redirect_code(0) {}
};

struct ServerConfig {
    static MimeType mimetype;
    std::vector<ListenConfig> listens;         // multiple interface:port pairs
    std::vector<std::string> server_names;     // a.com www.a.com
    std::string root;                          // /var/www/a
    std::vector<std::string> index;            // index.html | multiple index ??
    std::size_t client_max_body_size;          // bytes
    std::map<int, std::string> error_pages;    // 404 -> /errors/404.html
    std::vector<LocationConfig> locations;

    std::string errorPage(int code) const;

    ServerConfig() : client_max_body_size(1024 * 1024) {}
};

struct Config {
    std::vector<ServerConfig> servers;
};

#endif
