NAME    = webserv

CXX     = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

INCDIR  = -Iinclude/cgi \
		  -Iinclude/common \
		  -Iinclude/config \
		  -Iinclude/http \
		  -Iinclude/tcp

SRCS = src/config/Tokenizer.cpp \
	   src/config/ConfigParserServer.cpp \
	   src/config/ConfigParser.cpp \
	   src/config/ConfigParserUtils.cpp \
	   src/config/ConfigParserValidators.cpp \
	   src/config/ConfigParserLocation.cpp \
	   src/main.cpp \
	   src/tcp/ClientTable.cpp \
	   src/tcp/helper.cpp \
	   src/tcp/TcpServer.cpp \
	   src/tcp/EventLoop.cpp \
	   src/tcp/Socket.cpp \
	   src/tcp/SocketTable.cpp \
	   src/tcp/Client.cpp \
	   src/common/Logger.cpp


OBJS    = $(SRCS:.cpp=.o)

all: $(NAME)

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
