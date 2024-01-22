CC        =  g++
EXEC      =  binaries/prog
SRC       =  $(wildcard source/*.cpp)
SRC       += external/ImGuiFileDialog/ImGuiFileDialog.cpp
FLAGS     =  -lGLEW
FLAGS     += -Wall -Wextra -Wpedantic -std=c++20
FLAGS     += -Iexternal/glm -Iexternal/imgui 
FLAGS     += -Iexternal/imgui/backends -Iexternal/ImGuiFileDialog
OBJ       =  $(wildcard binaries/*.o)
IMGUI_SRC =  $(wildcard external/imgui/*.cpp)

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	FLAGS += -lGL -ldl `sdl2-config --libs --cflags`
endif
ifeq ($(UNAME_S), Darwin)
	FLAGS += -framework OpenGL -framework Cocoa 
	FLAGS += -framework IOKit -framework CoreVideo 
	FLAGS += `sdl2-config --libs --cflags`
	FLAGS += -I/usr/local/include -I/opt/local/include
endif
ifeq ($(UNAME_S), Windows_NT)
	FLAGS += -lgdi32 -lopengl32 -limm32 
	FLAGS += `pkg-config --static --libs --cflags sdl2`
endif

%.o:%.cpp
	$(CC) $(FLAGS) -c -o binaries/$@ $<
%.o:external/imgui/%.cpp
	$(CC) $(FLAGS) -c -o binaries/$@ $<
%.o:external/imgui/backends/%.cpp
	$(CC) $(FLAGS) -c -o binaries/$@ $<
%.o:external/imgui/misc/cpp/%.cpp
	$(CC) $(FLAGS) -c -o binaries/$@ $<

libs:
	mkdir -p binaries
	make imgui.o
	make imgui_draw.o
	make imgui_tables.o
	make imgui_widgets.o
	make imgui_impl_sdl.o 
	make imgui_impl_opengl3.o
	make imgui_demo.o
	make imgui_stdlib.o
build:
	mkdir -p binaries
	$(CC) $(SRC) $(OBJ) $(FLAGS) -o $(EXEC)
run:
	./binaries/prog
