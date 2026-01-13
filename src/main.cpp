#include <Gomoku.hpp>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

int main() {
  Gomoku game;
  Zobrist::initialize();
  game.run();
  return 0;
}
