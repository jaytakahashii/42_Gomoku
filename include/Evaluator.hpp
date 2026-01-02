#pragma once

#include "Board.hpp"

// 評価スコアの設定
// バランス調整をする際はこの数値を変更する
struct ScoreConfig {
  static constexpr int WIN = 100'000'000;
  static constexpr int OPEN_FOUR = 10'000'000;  // 棒四（両端空き）
  static constexpr int CLOSED_FOUR = 100'000;   // 四（片端空き）
  static constexpr int OPEN_THREE = 100'000;    // 三（両端空き）
  static constexpr int CAPTURE = 1'000'000;     // 捕獲の価値

  // 探索順序を決めるための簡易スコア（重み）
  static constexpr int PRIORITY_FIVE = 100'000;
  static constexpr int PRIORITY_FOUR = 10'000;
  static constexpr int PRIORITY_THREE = 1'000;
  static constexpr int PRIORITY_TWO = 100;
};

class Evaluator {
 public:
  // 盤面全体の静的評価（Minimaxの葉ノードで使用）
  static int evaluate(const Board& board, Color aiColor);

  // 手の生成時に、その手がどれくらい有望かを簡易評価する（Move Ordering用）
  static int evaluateMovePriority(const Board& board, int x, int y, Color color);

 private:
  // パターン認識ヘルパー
  static int _countPatterns(const BoardType& stones, const BoardType& empty);
};
