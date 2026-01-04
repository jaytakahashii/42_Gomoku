#pragma once

#include "Board.hpp"

// 評価スコアの設定
// バランス調整をする際はこの数値を変更する
struct ScoreConfig {
  static constexpr int WIN = 100'000'000;

  // --- 盤面評価用スコア ---
  static constexpr int OPEN_FOUR = 30'000'000;
  static constexpr int CLOSED_FOUR = 15'000'000;
  static constexpr int OPEN_THREE = 10'000'000;
  static constexpr int BROKEN_THREE = 500'000;
  static constexpr int CLOSED_THREE = 100'000;

  // ★追加・強化: 二連の評価
  // これを高めることで「相手の二連」を見つけたときに「これを放置すると相手のスコアが爆増するぞ！」とAIが危機感を持ちます。
  static constexpr int OPEN_TWO = 300'000;

  static constexpr int CAPTURE = 1'000'000;

  // --- 探索順序用スコア (Move Ordering) ---
  static constexpr int PRIORITY_WIN_BLOCK = 50'000'000;
  static constexpr int PRIORITY_FIVE = 10'000'000;
  static constexpr int PRIORITY_FOUR_BLOCK = 20'000'000;
  static constexpr int PRIORITY_FOUR = 5'000'000;

  // ★修正: 相手の「2」を防ぐ優先度を、自分の「2」を作るよりも高くする
  static constexpr int PRIORITY_THREE_BLOCK = 2'000'000;  // 相手の2(次3になるやつ)を防ぐ
  static constexpr int PRIORITY_THREE = 500'000;          // 自分の3を作る
  static constexpr int PRIORITY_TWO = 100'000;
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
