#pragma once

constexpr int BOARD_SIZE = 19;
constexpr int BOARD_WIDTH = 32;
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
};
