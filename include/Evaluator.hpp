#pragma once

#include "Board.hpp"

// 評価スコアの設定
// バランス調整をする際はこの数値を変更する
struct ScoreConfig {
  static constexpr int WIN = 1'000'000'000;

  // --- 盤面評価用スコア ---
  // Priority S+
  static constexpr int OPEN_FOUR = 100'000;

  // Priority S
  static constexpr int CLOSED_FOUR = 50'000;

  // Priority A
  static constexpr int OPEN_THREE = 2'500;

  // Priority B
  static constexpr int OPEN_TWO = 500;
  static constexpr int CAPTURE = 500;

  // Priority C
  static constexpr int CAPTURE_COUNT = 10;

  // --- 探索順序用スコア (Move Ordering) ---

  // [Priority S]
  static constexpr int PRIORITY_MAYBE_OPP_WIN = 500'000;

  // [Priority A+]
  static constexpr int PRIORITY_OPP_OPEN_THREE = 250'000;

  // [Priority A-]
  static constexpr int PRIORITY_MAYBE_MY_WIN = 250'000;

  // [Priority B]
  static constexpr int PRIORITY_CAPTURE = 150'000;
  static constexpr int PRIORITY_MY_OPEN_FOUR = 100'000;

  // [Priority C]
  static constexpr int PRIORITY_OPP_CLOSED_THREE = 75'000;
  static constexpr int PRIORITY_MY_CLOSED_FOUR = 50'000;
  static constexpr int PRIORITY_MY_OPEN_THREE = 50'000;
};

class Evaluator {
 public:
  // 盤面全体の静的評価（Minimaxの葉ノードで使用）
  static int evaluate(const Board& board, Color aiColor);

  // 手の生成時に、その手がどれくらい有望かを簡易評価する（Move Ordering用）
  static int evaluateMovePriority(const Board& board, int x, int y, Color color);

 private:
  // パターン認識ヘルパー
  static constexpr int SHIFT_H = 1;                 // 横
  static constexpr int SHIFT_V = BOARD_WIDTH;       // 縦 (20)
  static constexpr int SHIFT_D1 = BOARD_WIDTH + 1;  // 右下 (21)
  static constexpr int SHIFT_D2 = BOARD_WIDTH - 1;  // 左下 (19)
  static constexpr std::array<int, 4> ALL_DIRS = {SHIFT_H, SHIFT_V, SHIFT_D1, SHIFT_D2};

  static int _CountPatterns(const BoardType& myBoard, const BoardType& empty,
                            const BoardType& oppBoard);
};
