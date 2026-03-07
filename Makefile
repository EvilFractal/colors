CXX = g++
PKG = gtk4

# Compiler/linker flags
CXXFLAGS := -g -O0 -std=c++20 $(shell pkg-config --cflags $(PKG))
LDFLAGS := $(shell pkg-config --libs $(PKG))
SRCS = colorpicker.cpp
OBJS = $(SRCS:*.cpp=*)
TARGET = bin/colorpicker
all: build
build: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)
run: build
	./$(TARGET)
