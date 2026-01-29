CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Isrc
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = src/main.cpp src/game/player.cpp src/game/world.cpp
OUT = game

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT)
