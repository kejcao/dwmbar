CXX := g++
CXXFLAGS := -std=c++20 -pedantic -Wall -O3

LIBS := -I/usr/include/libdrm -ldrm -ldrm_amdgpu -lX11 -lsensors

SRC := $(wildcard *.cpp)
HEADERS := $(wildcard modules/*.h)
TARGET := dwmbar

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(LIBS) $(SRC) -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean

