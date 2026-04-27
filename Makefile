NAME    = webserv
CONFIG_FILE = config_file

CXX     = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g


DEPS = \
	   ./include/common \
	   ./include/config \
	   ./include/http/common \
	   ./include/http/message \
	   ./include/http/parser \
	   ./include/http/types \
	   ./include/router \
	   ./include/server 

INCDIR  = $(addprefix -I,$(DEPS))

SRCS = \
	   ./src/http/common/MimeType.cpp \
	   ./src/common/Logger.cpp \
	   ./src/config/ConfigParser.cpp \
	   ./src/config/ConfigParserListen.cpp \
	   ./src/config/ConfigParserLocation.cpp \
	   ./src/config/ConfigParserServer.cpp \
	   ./src/config/ConfigParserServerDirectives.cpp \
	   ./src/config/ConfigParserSharedDirectives.cpp \
	   ./src/config/ConfigParserUtils.cpp \
	   ./src/config/ConfigParserValidators.cpp \
	   ./src/config/ServerConfig.cpp \
	   ./src/config/Tokenizer.cpp \
	   ./src/server/Cgi.cpp \
	   ./src/server/Client.cpp \
	   ./src/server/ClientTable.cpp \
	   ./src/server/EventLoop.cpp \
	   ./src/server/FileServe.cpp \
	   ./src/server/HttpServer.cpp \
	   ./src/server/Socket.cpp \
	   ./src/server/SocketTable.cpp \
	   ./src/server/helper.cpp \
	   ./src/main.cpp \
	   ./src/http/common/BodyStorage.cpp \
	   ./src/http/common/Buffer.cpp \
	   ./src/http/message/HttpMessage.cpp \
	   ./src/http/message/HttpRequest.cpp \
	   ./src/http/message/HttpResponse.cpp \
	   ./src/http/parser/HttpParserState.cpp \
	   ./src/http/parser/HttpParserState_body.cpp \
	   ./src/http/parser/HttpParserState_headers_parse.cpp \
	   ./src/http/parser/HttpParserState_headers_process.cpp \
	   ./src/http/parser/HttpRequestParser.cpp \
	   ./src/http/parser/HttpRequestParser_request_line_parse.cpp \
	   ./src/http/parser/HttpRequestParser_status_process.cpp \
	   ./src/http/parser/HttpResponseParser.cpp \
	   ./src/http/parser/HttpResponseParser_headers_parse.cpp \
	   ./src/http/parser/HttpResponseParser_headers_process.cpp \
	   ./src/http/types/Method.cpp \
	   ./src/http/types/ParserError.cpp \
	   ./src/http/types/Status.cpp \
	   ./src/router/Router.cpp \
	   ./src/router/RouterResolver.cpp

OBJS    = $(SRCS:.cpp=.o)

all: $(NAME)

push: fclean
	read -p "commit message: " msg; \
		git add .; \
		git commit -am "$$msg"; \
		git push

runq: all
	./$(NAME) $(CONFIG_FILE)

runqv: all
	valgrind ./$(NAME) $(CONFIG_FILE)

run: re
	make clean
	./$(NAME) $(CONFIG_FILE)

runv: re
	make clean
	valgrind ./$(NAME) $(CONFIG_FILE)

runvfd: re
	make clean
	valgrind --track-fds=yes ./$(NAME) $(CONFIG_FILE)


$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
