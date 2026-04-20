NAME    = webserv
CONFIG_FILE = config_file

CXX     = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

INCDIR  = -Iinclude/cgi \
		  -Iinclude/common \
		  -Iinclude/config \
		  -Iinclude/http \
		  -Iinclude/request \
		  -Iinclude/parser \
		  -Iinclude/buffer \
		  -Iinclude/server

SRCS = ./src/config/ConfigParserSharedDirectives.cpp \
	   ./src/config/Tokenizer.cpp \
	   ./src/config/ConfigParserServer.cpp \
	   ./src/config/ConfigParser.cpp \
	   ./src/config/ConfigParserUtils.cpp \
	   ./src/config/ConfigParserValidators.cpp \
	   ./src/config/ConfigParserServerDirectives.cpp \
	   ./src/config/ConfigParserLocation.cpp \
	   ./src/config/ConfigParserListen.cpp \
	   ./src/main.cpp \
	   ./src/request/HttpResponse.cpp \
	   ./src/request/HttpRequest.cpp \
	   ./src/request/Status.cpp \
	   ./src/request/HttpMessage.cpp \
	   ./src/request/Method.cpp \
	   ./src/parser/HttpParserBody.cpp \
	   ./src/parser/BodyStorage.cpp \
	   ./src/parser/parse_request_line.cpp \
	   ./src/parser/Error.cpp \
	   ./src/parser/HttpParser.cpp \
	   ./src/parser/HttpParserRequestLine.cpp \
	   ./src/parser/parse_header_line.cpp \
	   ./src/parser/HttpParserState.cpp \
	   ./src/parser/HttpParserHeaders.cpp \
	   ./src/server/ClientTable.cpp \
	   ./src/server/helper.cpp \
	   ./src/server/HttpServer.cpp \
	   ./src/server/SocketTable.cpp \
	   ./src/server/EventLoop.cpp \
	   ./src/server/Socket.cpp \
	   ./src/server/Client.cpp \
	   ./src/server/FileServe.cpp \
	   ./src/server/Cgi.cpp \
	   ./src/common/Logger.cpp \
	   ./src/buffer/Buffer.cpp


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
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
