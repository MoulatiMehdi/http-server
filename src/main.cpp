#include "HttpParser.hpp"

#include "HttpRequest.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

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
            "Transfer-encoding:       chunked     \r\n",
            "CONTENT-TYPE",
            ": application/json\r",
            "\n",
            "",
            "type-encoding:\r\nhello:world\r\n",
            "\r\n10\r\n",
            "0123456789abcdef\r\n0\r\n\r\nPOST /  HTTP/1.0\r\ncontent-length: ",
            "0\r\n\r\n"
        };
        const int size = sizeof(tokens) / sizeof(tokens[0]);

        for (int i = 0; i < size; i++)
        {
            parser.parse(request, tokens[i].c_str(), tokens[i].size());
            if (request.complete())
            {
                std::cout << request << std::endl;
                std::cout << parser << std::endl;
                parser.clear();
                request.clear();
            }
            if (!request.good())
            {
                std::cout << request << std::endl;
                std::cout << parser << std::endl;

                return -1;
            }
        }
    }

    return 0;
}
