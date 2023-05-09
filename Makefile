main:
	g++ -c nbody.cpp
	g++ nbody.o ./imgui/build/*.o -lGLEW -lglfw -lGL
release:
	g++ -c nbody.cpp -O3
	g++ nbody.o ./imgui/build/*.o -lGLEW -lglfw -lGL
build:
	cd ./imgui; g++ -c -O3 ./*.cpp
	mkdir ./imgui/build
	cp ./imgui/*.o ./imgui/build

