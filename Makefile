CXXFLAGS = -Wall -Wextra -Werror -std=c++17
SRCS     = ./sample.cpp
NAME     = Gomoku
OBJS     = $(SRCS:.cpp=.o)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
    SFML_PATH = /opt/homebrew
else
    SFML_PATH = $(shell brew --prefix)
endif

IFLAGS  = -I$(SFML_PATH)/include
LDFLAGS = -L$(SFML_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
