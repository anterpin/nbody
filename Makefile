main:
	g++ -c nbody.cpp -Wall -g -lm -m64
	g++ nbody.o ./imgui/build/*.o -lGLEW -lglfw -lGL
release:
	g++ -c nbody.cpp -O3 -m64
	g++ nbody.o ./imgui/build/*.o -lGLEW -lglfw -lGL
build:
	cd ./imgui; g++ -c -O3 ./*.cpp -m64
	mkdir ./imgui/build
	mv ./imgui/*.o ./imgui/build
win:
	x86_64-w64-mingw32-g++ nbody.cpp -O3 -o windows/nbody.exe imgui/*.cpp -I./windows/glew-2.1.0/include/ -I./windows/glm -I./windows/glfw-3.3.8.bin.WIN64/include/ windows/glew-2.1.0/lib/Release/x64/glew32.lib windows/glfw-3.3.8.bin.WIN64/lib-mingw-w64/*.a -lgdi32 -lopengl32 -lglu32 -lkernel32 -luser32 -static-libgcc -static-libstdc++ -static -lwinpthread


