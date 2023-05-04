main:
	g++ -c nbody.cpp
	g++ nbody.o ./imgui/build/*.o -lGLEW -lglfw -lGL
build:
	g++ -c ./imgui/*.cpp
	cp ./imgui/*.o ./imgui/build

