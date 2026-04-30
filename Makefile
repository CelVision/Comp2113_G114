# Makefile for Tower Defense Game

CXX = g++
CXXFLAGS = -std=c++11 -Wall
TARGET = Game.exe
SRC = Game.cpp

all: $(TARGET)

$(TARGET): $(SRC)
    $(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
    ./$(TARGET)

clean:
    rm -f $(TARGET)

.PHONY: all run clean