CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Isrc
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRC = src/main.cpp src/game/player.cpp src/game/world.cpp src/game/view.cpp src/game/mask.cpp src/game/ui.cpp
OUT = game

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT)
