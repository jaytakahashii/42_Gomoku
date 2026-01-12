#pragma once

#include <array>
#include <cstdint>
#include <random>

#include "Enums.hpp"
#include "GameConfig.hpp"

class Zobrist {
 public:
  // Initialize the Zobrist tables with random values once
  static void initialize() {
    std::mt19937_64 rng(12345);  // Fixed seed for reproducibility
    std::uniform_int_distribution<uint64_t> dist;

    for (int i = 0; i < MAX_CELLS; ++i) {
      table[i][0] = dist(rng);  // Black
      table[i][1] = dist(rng);  // White
    }
    blackTurn = dist(rng);
  }

  // Get the random bitstring for a specific position and color
  // colorIndex: 0 for Black, 1 for White
  static uint64_t getPieceHash(int index, Color color) {
    return table[index][(color == Color::BLACK) ? 0 : 1];
  }

  static uint64_t getBlackTurnHash() {
    return blackTurn;
  }

 private:
  // [Position][Color]
  static inline uint64_t table[MAX_CELLS][2];
  static inline uint64_t blackTurn;
};
