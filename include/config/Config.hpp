#ifndef   CONFIG_HPP
# define  CONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct LocationConfig {
    std::string path;                          // "/"
    std::vector<std::string> allowed_methods;  // GET POST DELETE
    std::string root;                          // optional override
    std::vector<std::string> index;            // e.g. index.html
    bool autoindex;                            // true/false
    std::string cgi_extension;                 // e.g. ".py"
    std::string cgi_path;                      // e.g. /usr/bin/python3
    std::string upload_dir;                    // e.g. /tmp/uploads
    size_t client_max_body_size;               // bytes, 0 => inherit
    int redirect_code;                         // 0 => none, else 301/302... return
    std::string redirect_url;                  // redirect target            return

    LocationConfig() : autoindex(false), client_max_body_size(0), redirect_code(0) {}
};

struct ServerConfig {
    int listen_port;                           // 8080
    std::string listen_host;                   // 
    std::vector<std::string> server_names;     // a.com www.a.com
    std::string root;                          // /var/www/a
    std::vector<std::string> index;            // index.html | multiple index ??
    size_t client_max_body_size;               // bytes
    std::map<int, std::string> error_pages;    // 404 -> /errors/404.html
    std::vector<LocationConfig> locations;
    ServerConfig() : listen_port(80), listen_host("0.0.0.0"),
                     client_max_body_size(1024 * 1024) {}
};

struct Config {
    std::vector<ServerConfig> servers;
};

#endif
