NAME        = Gomoku
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++17

LOCAL_PATH  = $(shell pwd)/sfml_install
IFLAGS      = -I$(LOCAL_PATH)/include
LDFLAGS     = -L$(LOCAL_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system
RPATH       = -Wl,-rpath,$(LOCAL_PATH)/lib

SRCS        = ./sample.cpp
OBJS        = $(SRCS:.cpp=.o)

all: $(LOCAL_PATH) $(NAME)

$(LOCAL_PATH):
	@echo "SFML not found. Downloading and building locally..."
	git clone --depth 1 -b 3.0.0 https://github.com/SFML/SFML.git sfml_src
	cmake -S sfml_src -B sfml_build -DCMAKE_INSTALL_PREFIX=$(LOCAL_PATH) -DSFML_BUILD_EXAMPLES=OFF
	cmake --build sfml_build --target install -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu)
	rm -rf sfml_src sfml_build

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS) $(RPATH)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	rm -rf $(LOCAL_PATH)

re: fclean all

.PHONY: all clean fclean re
