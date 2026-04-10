NAME    = webserv
CONFIG_FILE = config_file

CXX     = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

INCDIR  = -Iinclude/cgi \
		  -Iinclude/common \
		  -Iinclude/config \
		  -Iinclude/http \
		  -Iinclude/server

SRCS = src/config/Tokenizer.cpp \
	   src/config/ConfigParserServer.cpp \
	   src/config/ConfigParser.cpp \
	   src/config/ConfigParserUtils.cpp \
	   src/config/ConfigParserValidators.cpp \
	   src/config/ConfigParserLocation.cpp \
	   src/main.cpp \
	   src/server/ClientTable.cpp \
	   src/server/helper.cpp \
	   src/server/HttpServer.cpp \
	   src/server/EventLoop.cpp \
	   src/server/Socket.cpp \
	   src/server/SocketTable.cpp \
	   src/server/Client.cpp \
	   src/common/Logger.cpp


OBJS    = $(SRCS:.cpp=.o)

all: $(NAME)

push: fclean
	read -p "commit message: " msg; \
		git add .; \
		git commit -am "$$msg"; \
		git push

runq: all
	./$(NAME) $(CONFIG_FILE)

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
