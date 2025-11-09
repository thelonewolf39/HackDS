# Makefile for HackDS C++ Game Example

CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
LDFLAGS = -lSDL2

TARGET = game
SOURCES = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

# For cross-compilation to Raspberry Pi
arm64: CXX = aarch64-linux-gnu-g++
arm64: $(TARGET)

.PHONY: all clean arm64
