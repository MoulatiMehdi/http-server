#include "Debug.hpp"
#include "HttpMessage.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "MimeType.hpp"
#include "Status.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
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
    // int fd_server = server_open();

    // char buffer[1024];
    // while (true)
    // {
    //     HttpRequest request;
    //
    //     int fd_client = accept(fd_server, 0, 0);
    //     if (fd_client < 0)
    //     {
    //         perror("accept");
    //         return 0;
    //     }
    //     ssize_t rsize = read(fd_client, &buffer, 1024);
    //     if (rsize < 0)
    //         perror("read");
    //     if (rsize > 0)
    //         request.parse(buffer, rsize);
    //     HttpResponse response;
    //     std::string  str;
    //     print_request(request);
    //     if (!request.good())
    //     {
    //         response.setStatus(request.status());
    //         str = response.serve_page();
    //     }
    //     else if (request.complete())
    //     {
    //         response.setStatus(request.status());
    //         str = response.serve_directory(
    //             "/home/mmoulati", request.uri().c_str()
    //         );
    //         if (!response.good())
    //             str = response.serve_page();
    //     }
    //     else
    //     {
    //         response.setStatus(status::GATEWAY_TIMEOUT);
    //         str = response.serve_page();
    //     }
    //     write(fd_client, str.c_str(), str.size());
    //     print_string_nl(str);
    //     close(fd_client);
    // }
    //

    MimeType mm;

    std::cout << mm.getContentType("txt") << std::endl;
    std::cout << mm.getContentType("pdf") << std::endl;
    return 0;
}
