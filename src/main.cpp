#include "HttpParser.hpp"

#include "HttpRequest.hpp"
#include <cstring>
#include <iostream>

int main()
{
    HttpRequest request;
    HttpParser  parser;
    {
        std::string tokens[] = {
            "GET /        HTTP/1.0\r\n",
            "CONTENT-LENGTH",
            ": 100\r",
            "\n",
            "CONTENT-TYPE",
            ": application/json\r",
            "\n",
            "",
            "type-encoding:\r\nhello:world\r\n\r\nthanks yout for every think"
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
    return 0;
}
