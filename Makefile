dwmbar: main.cpp
	g++ -std=c++20 $(shell pkg-config --cflags --libs libdrm) -ldrm_amdgpu -lX11 -lsensors -pedantic -Wall -O3 $^ -o $@
