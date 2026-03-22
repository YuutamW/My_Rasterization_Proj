# --- Variables ---
CC = g++

CFLAGS = -std=c++17 -Wall -g -O3 -fopenmp $(shell pkg-config --cflags sdl3)

LDFLAGS = -fopenmp $(shell pkg-config --libs sdl3)

SRC = main.cpp display.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = rasterizer

# --- Rules ---

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 2. Added "time" here. This measures how long the executable runs.
run: $(TARGET)
	time ./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)