MAKEFLAGS += -j$(shell nproc)

NAME = engine

IMGUI_DIR = external/imgui
TINYGLTF_DIR = external/tinygltf

CPPSRCS = main.cpp \
	engine.cpp \
	window.cpp \
	renderer.cpp \
	imgui_layer.cpp \
	camera.cpp \
	component.cpp \
	transform_component.cpp \
	mesh_component.cpp \
	entity.cpp \
	model.cpp \
	tinygltfloader.cpp \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/imgui_impl_glfw.cpp \
	$(IMGUI_DIR)/imgui_impl_vulkan.cpp

CSRCS = $(TINYGLTF_DIR)/tiny.c \
		$(TINYGLTF_DIR)/tiny_gltf_v3.c 

OBJS = $(CPPSRCS:.cpp=.o) $(CSRCS:.c=.o)

CC = cc
CFLAGS = -std=c11 -isystem external -I$(TINYGLTF_DIR) -DTINYGLTF3_ENABLE_FS

CXX = c++
CXXFLAGS = -std=c++20 -isystem external -I$(IMGUI_DIR)
CXXFLAGS += -g -Wall -Wformat
CXXFLAGS += `pkg-config --cflags glfw3 vulkan`

LIBS = `pkg-config --libs glfw3 vulkan` -lGL



all: $(NAME)

$(NAME): $(OBJS)
	bash shaders/compile.sh
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean
	all

debug: CXXFLAGS += -O0 -g3 -DDEBUG
debug: re