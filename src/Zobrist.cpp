#include "Zobrist.hpp"

uint64_t Zobrist::table[MAX_CELLS][2];
uint64_t Zobrist::blackTurn;

void Zobrist::initialize() {
  std::mt19937_64 rng(12345);
  std::uniform_int_distribution<uint64_t> dist;

  for (int i = 0; i < MAX_CELLS; ++i) {
    table[i][0] = dist(rng);
    table[i][1] = dist(rng);
  }
  blackTurn = dist(rng);
}

uint64_t Zobrist::getPieceHash(int index, Color color) {
  int colorIndex = (color == Color::BLACK) ? 0 : 1;
  return table[index][colorIndex];
}

uint64_t Zobrist::getBlackTurnHash() {
  return blackTurn;
}
