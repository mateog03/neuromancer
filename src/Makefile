CC = g++
SFLAGS = -Wall -Wextra -Wpedantic -std=c++20
OFLAGS = -O3 -march=native -flto -DNDEBUG
LFLAGS = -lpthread
SRC = $(wildcard *.cpp)
BIN = neuromancer

all:
	$(CC) $(SFLAGS) $(OFLAGS) $(SRC) $(LFLAGS) -o $(BIN)
