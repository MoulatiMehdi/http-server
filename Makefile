CXX = c++ 
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g 

DIRS := $(shell find -name "*.hpp" | xargs dirname | sort | uniq)
DEPS := $(addprefix -I,$(DIRS))

BUILD_DIR := build
SRCS := $(shell find -name "*.cpp" | grep -v test  | sort | uniq)



OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRCS))

NAME = run 


all :  $(NAME)

$(NAME) : $(OBJS) | $(BUILD_DIR) 
	$(CXX) $(CXXFLAGS) $(DEPS)  $(OBJS) -o $(NAME) 

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -g $(DEPS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean :
	rm -rf $(BUILD_DIR)

fclean : clean 
	rm -rf $(NAME)

re : fclean all

.PHONY: all  clean fclean re 
.SECONDARY: $(OBJS)
