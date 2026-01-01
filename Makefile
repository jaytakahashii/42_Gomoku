NAME = Gomoku
CXX  = c++
#CXXFLAGS = -Wall -Wextra -Werror -std=c++17
CXXFLAGS = -std=c++17

ROOT := $(shell pwd)

SFML_SRC_PATH   = $(ROOT)/sfml_src
SFML_BUILD_PATH = $(ROOT)/sfml_build
SFML_LOCAL_PATH = $(ROOT)/sfml_install

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)

	SFML_LIB = $(SFML_LOCAL_PATH)/lib/libsfml-graphics.so

	IFLAGS  = -I$(SFML_LOCAL_PATH)/include
	LDFLAGS = -L$(SFML_LOCAL_PATH)/lib \
	          -lsfml-graphics -lsfml-window -lsfml-system

	RPATH   = -Wl,-rpath,$(SFML_LOCAL_PATH)/lib

	CMAKE_OPTS = \
	  -DBUILD_SHARED_LIBS=ON \
	  -DSFML_BUILD_AUDIO=OFF \
	  -DSFML_BUILD_EXAMPLES=OFF

endif

ifeq ($(UNAME_S),Darwin)

	SFML_LIB = $(SFML_LOCAL_PATH)/lib/libsfml-graphics.dylib

	IFLAGS  = -I$(SFML_LOCAL_PATH)/include
	LDFLAGS = -L$(SFML_LOCAL_PATH)/lib \
	          -lsfml-graphics -lsfml-window -lsfml-system \
	          -framework OpenGL \
	          -framework Cocoa \
	          -framework IOKit \
	          -framework CoreFoundation \
	          -framework CoreVideo

	RPATH   = -Wl,-rpath,$(SFML_LOCAL_PATH)/lib

	CMAKE_OPTS = \
	  -DBUILD_SHARED_LIBS=ON \
	  -DSFML_BUILD_AUDIO=OFF \
	  -DSFML_BUILD_EXAMPLES=OFF

endif

ifneq ($(filter $(UNAME_S),Darwin Linux),$(UNAME_S))
    $(error This Makefile only supports macOS (Darwin) and Ubuntu (Linux). Detected: $(UNAME_S))
endif

SRC_DIR            = src/
OBJ_DIR            = .obj/
SRC_FILES          = main.cpp \
					 Gomoku.cpp \
					 MenuScene.cpp \
					 GameScene.cpp \
					 ResultScene.cpp \
					 Board.cpp \
					 AI.cpp

IFLAGS            += -Iinclude
SRCS               = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJS               = $(addprefix $(OBJ_DIR), $(SRC_FILES:.cpp=.o))

all: $(SFML_LIB) $(NAME)

$(SFML_LIB):
	@echo "Building SFML locally for $(UNAME_S)..."
	rm -rf $(SFML_SRC_PATH) $(SFML_BUILD_PATH) $(SFML_LOCAL_PATH)
	git clone --depth 1 -b 3.0.0 https://github.com/SFML/SFML.git $(SFML_SRC_PATH)
	cmake -S $(SFML_SRC_PATH) -B $(SFML_BUILD_PATH) \
	-DCMAKE_INSTALL_PREFIX=$(SFML_LOCAL_PATH) \
	$(CMAKE_OPTS)
	cmake --build $(SFML_BUILD_PATH) --target install -j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu)
	rm -rf $(SFML_SRC_PATH) $(SFML_BUILD_PATH)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(RPATH)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	rm -rf $(SFML_SRC_PATH) $(SFML_BUILD_PATH) $(SFML_LOCAL_PATH)

re: fclean all

run: all
	./$(NAME)

.PHONY: all clean fclean re run
