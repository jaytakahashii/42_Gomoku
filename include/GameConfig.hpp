#pragma once

#include <array>
#include <bitset>

constexpr int BOARD_SIZE = 19;
constexpr int BOARD_WIDTH = 32;  // 19 + sentinel padding
constexpr int MAX_CELLS = BOARD_WIDTH * BOARD_SIZE;
constexpr int CENTER_INDEX = (BOARD_SIZE / 2) * BOARD_WIDTH + (BOARD_SIZE / 2);
// (9, 9) -> (9, 6)
constexpr int PRO_CLOSEST_TOP_INDEX = CENTER_INDEX - (BOARD_WIDTH * 3);     // 201
constexpr int PRO_CLOSEST_BOTTOM_INDEX = CENTER_INDEX + (BOARD_WIDTH * 3);  // 337
// (9, 9) -> (9, 5)
constexpr int LONG_PRO_CLOSEST_TOP_INDEX = CENTER_INDEX - (BOARD_WIDTH * 4);     // 169
constexpr int LONG_PRO_CLOSEST_BOTTOM_INDEX = CENTER_INDEX + (BOARD_WIDTH * 4);  // 369

constexpr int WIDTH_SHIFT = 5;
constexpr int WIDTH_MASK = 31;  // 0x1F

constexpr std::array<int, 4> DIR_OFFSETS = {
    1,                // Horizontal
    BOARD_WIDTH,      // Vertical
    BOARD_WIDTH + 1,  // Diagonal (Top-Left to Bottom-Right)
    BOARD_WIDTH - 1   // Diagonal (Top-Right to Bottom-Left)
};

// ==========================================
// Move Structure
// ==========================================

struct Move {
  int index;
  int score;

  bool operator>(const Move& other) const {
    return score > other.score;
  }
  bool operator==(const Move& other) const {
    return index == other.index;
  }
};
