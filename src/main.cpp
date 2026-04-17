#include "Debug.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponse.hpp"
#include "Status.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int server_open()
{
    // create a socket for the server
    const int fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_server < 0)
    {
        perror("socket");
        return -1;
    }
    int optval = 1;
    setsockopt(
        fd_server,
        SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT,
        &optval,
        sizeof(optval)
    );

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    // reuse an address port

    addr.sin_family      = AF_INET;     // IPV4
    addr.sin_addr.s_addr = INADDR_ANY;  // ANY ADDRESS
    addr.sin_port        = htons(8080); // PORT 80

    if (bind(fd_server, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return -1;
    }

    if (listen(fd_server, 128) < 0)
    {
        perror("listen");
        return -1;
    }
    std::cout << "http://" << inet_ntoa(addr.sin_addr) << ":"
              << ntohs(addr.sin_port) << std::endl;
    return fd_server;
}

int main()
{
    int fd_server = server_open();

    char buffer[1024];
    while (true)
    {
        HttpRequest       request;
        HttpRequestParser parser(request);

        int fd_client = accept(fd_server, 0, 0);
        if (fd_client < 0)
        {
            perror("accept");
            return 0;
        }
        ssize_t rsize = read(fd_client, &buffer, 1024);
        if (rsize < 0)
            perror("read");
        if (rsize > 0)
        {
            std::cout << "--------------------- Buffer ----------------------- "
                      << std::endl;
            print_ptr_nl(buffer, rsize);
            parser.parse(buffer, rsize);
        }
        HttpResponse       response;
        std::string        str;
        std::ostringstream oss("", std::_S_app);
        std::cout << "--------------------- Request ----------------------- "
                  << std::endl;
        print_request(request);
        if (!request.good())
        {
            response.setStatus(request.status());
            oss << "<html><head>";
            oss << "<title>" << phrase_reason(response.status()) << "</title>";
            oss << "</head><body>";
            oss << "<h1>" << phrase_reason(response.status())
                << "</h1></body></html>";

            str = oss.str();
            oss.str("");
            oss.clear();
            oss << str.size();
        }
        else if (request.complete())
        {
            response.setStatus(request.status());
            str =
                "<html><head><title>hello world</title></head><body><h1>Hello "
                "World</h1></body></html>";
            oss.str("");
            oss.clear();
            oss << str.size();
        }
        else
        {
            response.setStatus(status::GATEWAY_TIMEOUT);
            oss << "<html><head>";
            oss << "<title>" << phrase_reason(response.status()) << "</title>";
            oss << "</head><body>";
            oss << "<h1>" << phrase_reason(response.status())
                << "</h1></body></html>";

            str = oss.str();
            oss.str("");
            oss.clear();
            oss << str.size();
        }
        response.setHeader("content-length", oss.str());
        response.setHeader("content-type", "text/html");
        response.setHeader("connection", "close");
        const std::string res = response.to_string();
        write(fd_client, res.c_str(), res.size());
        write(fd_client, str.c_str(), str.size());
        std::cout << "--------------------- Response ----------------------- "
                  << std::endl;
        print_response(response);
        std::cout << parser << std::endl;
        close(fd_client);
    }
    return 0;
}
