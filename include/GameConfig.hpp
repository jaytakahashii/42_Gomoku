#pragma once

#include <array>
#include <bitset>

// 基本定数
constexpr int BOARD_SIZE = 19;
constexpr int BOARD_WIDTH = 32;  // 19 + sentinel padding
constexpr int MAX_CELLS = BOARD_WIDTH * BOARD_SIZE;
constexpr int CENTER_INDEX = (BOARD_SIZE / 2) * BOARD_WIDTH + (BOARD_SIZE / 2);

// ビット演算用
constexpr int WIDTH_SHIFT = 5;
constexpr int WIDTH_MASK = 31;  // 0x1F

// 重要: ここで1回だけ定義する（Single Source of Truth）
// AI, Board, Evaluator すべてがこれを使う
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

using BoardType = std::bitset<MAX_CELLS>;
