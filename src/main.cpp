
#include <iostream>
#include "Config.hpp"
#include "ConfigParser.hpp"

void printConfig(const Config &config) {
	for (std::size_t s = 0; s < config.servers.size(); s++) {
		const ServerConfig &srv = config.servers[s];
		std::cout << "server [" << s << "] {\n";
		std::cout << "  listen:              " << srv.listen_host << ":"
				  << srv.listen_port << "\n";
		std::cout << "  root:                " << srv.root << "\n";
		std::cout << "  client_max_body:     " << srv.client_max_body_size
				  << "\n";

		std::cout << "  server_names:        ";
		for (std::size_t i = 0; i < srv.server_names.size(); i++)
			std::cout << srv.server_names[i] << " ";
		std::cout << "\n";

		std::cout << "  index:               ";
		for (std::size_t i = 0; i < srv.index.size(); i++)
			std::cout << srv.index[i] << " ";
		std::cout << "\n";

		for (std::map<int, std::string>::const_iterator it =
				 srv.error_pages.begin();
			 it != srv.error_pages.end(); it++)
			std::cout << "  error_page:          " << it->first << " -> "
					  << it->second << "\n";

		for (std::size_t l = 0; l < srv.locations.size(); l++) {
			const LocationConfig &loc = srv.locations[l];
			std::cout << "  location [" << loc.path << "] {\n";
			std::cout << "    root:              " << loc.root << "\n";
			std::cout << "    autoindex:         "
					  << (loc.autoindex ? "on" : "off") << "\n";
			std::cout << "    client_max_body:   " << loc.client_max_body_size
					  << "\n";
			std::cout << "    cgi_extension:     " << loc.cgi_extension << "\n";
			std::cout << "    cgi_path:          " << loc.cgi_path << "\n";
			std::cout << "    upload_dir:        " << loc.upload_dir << "\n";

			std::cout << "    allowed_methods:   ";
			for (std::size_t i = 0; i < loc.allowed_methods.size(); i++)
				std::cout << loc.allowed_methods[i] << " ";
			std::cout << "\n";

			std::cout << "    index:             ";
			for (std::size_t i = 0; i < loc.index.size(); i++)
				std::cout << loc.index[i] << " ";
			std::cout << "\n";

			if (loc.redirect_code)
				std::cout << "    return:            " << loc.redirect_code
						  << " " << loc.redirect_url << "\n";
			std::cout << "  }\n";
		}
		std::cout << "}\n";
	}
}

#include "TcpServer.hpp"
int main(int ac, char **av) {
	if (ac != 2) {
		std::cout << "Invalide arguments" << std::endl;
		std::cout << "  Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}

	// try {
	// 	ConfigParser parser;
	// 	Config config;
	// 	config = parser.parseFile(av[1]);
	// 	printConfig(config);
	// 	return 0;
	// } catch (const std::exception &e) {
	// 	// std::cerr << e.what() << std::endl;
	// 	return 1;
	// }

	TcpServer server(av[1]); // TODO: rename to HttpServer

	// try catch
	server.init();
	server.run();
	return 0;
}
