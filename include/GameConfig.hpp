#pragma once

#include <bitset>

constexpr int BOARD_SIZE = 19;
constexpr int BOARD_WIDTH = 32;  // 19 + sentinel padding
constexpr int MAX_CELLS = BOARD_WIDTH * BOARD_SIZE;

// ==========================================
// Move Structure
// ==========================================

struct Move {
  int x;
  int y;
  int score;

  // Operator for sorting moves (descending order of score).
  // Crucial for Move Ordering in Alpha-Beta pruning.
  bool operator>(const Move& other) const {
    return score > other.score;
  }

  bool operator==(const Move& other) const {
    return x == other.x && y == other.y;
  }
};

// Type alias for the bit board representation
using BoardType = std::bitset<MAX_CELLS>;
