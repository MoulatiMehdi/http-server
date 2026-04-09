#include "HttpParser.hpp"

#include "HttpRequest.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>

int main()
{
    HttpRequest request;
    HttpParser  parser;
    {
        std::string tokens[] = {
            "POST /        HTTP/1.0\r\n",
            "CONTENT-LENGTH",
            ": 10   \r",
            "\n",
            // "Transfer-encoding:       chunked     \r\n",
            "CONTENT-TYPE",
            ": application/json\r",
            "\n",
            "",
            "type-encoding:\r\nhello:world\r\n",
            "\r\n",
            "thank you\r\n0\r\n\r\nt for every think"
        };
        const int size = sizeof(tokens) / sizeof(tokens[0]);

        for (int i = 0; i < size; i++)
        {
            parser.parse(request, tokens[i].c_str(), tokens[i].size());
        }
    }

    if (parser.good())
        std::cout << request << std::endl;
    else
    {
        std::cout << to_string(parser.error()) << std::endl;
        std::cout << to_string(parser.state()) << std::endl;
    }
    getchar();

    return 0;
}
