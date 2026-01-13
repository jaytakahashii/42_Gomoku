#include <Gomoku.hpp>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

Gomoku* g_pointer = nullptr;

void signalHandler(int sig) {
  g_pointer->stop();
}

int main() {
  Gomoku game;
  g_pointer = &game;
  std::signal(SIGINT, signalHandler);
  Zobrist::initialize();
  game.run();
  std::cout << "Gomoku was Terminated." << std::endl;
  return 0;
}
