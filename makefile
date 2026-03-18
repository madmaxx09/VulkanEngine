MAKEFLAGS += -j$(shell nproc)

NAME = engine

IMGUI_DIR = external/imgui
OBJ_DIR = objs

SRCS = main.cpp \
	engine.cpp \
	window.cpp \
	renderer.cpp \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/imgui_impl_glfw.cpp \
	$(IMGUI_DIR)/imgui_impl_vulkan.cpp

OBJS = $(SRCS:.cpp=.o)

CXX = c++
CXXFLAGS = -std=c++20 -I$(IMGUI_DIR)
CXXFLAGS += -g -Wall -Wformat
CXXFLAGS += `pkg-config --cflags glfw3 vulkan`

LIBS = `pkg-config --libs glfw3 vulkan` -lGL



all: $(NAME)

$(NAME): $(OBJS)
	bash shaders/compile.sh
	$(CXX) -o $@ $^ $(LIBS)
	rm *.o

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CXXFLAGS += -O0 -g3 -DDEBUG
debug: re