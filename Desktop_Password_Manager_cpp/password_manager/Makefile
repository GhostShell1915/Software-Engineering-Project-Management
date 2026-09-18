# Simple Makefile alternative to CMake
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS  ?= -lcrypto

SRCS = src/main.cpp src/crypto.cpp src/vault.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = password_manager

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) vault.dat
