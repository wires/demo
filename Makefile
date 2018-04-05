demo: main.cc
	clang++ -Oz `pkg-config --cflags --libs glfw3 gl` main.cc -o demo
