#pragma once

#include <cmath>

#include "Board.hpp"

// 評価スコアの設定
struct ScoreConfig {
  static constexpr int WIN = 100'000'000;  // 確実な勝利

  // --- 盤面評価用 (Static Evaluation) ---
  // 基本的に (Open > Closed) かつ (Length N > Length N-1)

  static constexpr int OPEN_FOUR = 100'000;   // 次に5連にできる (Win確約)
  static constexpr int CLOSED_FOUR = 10'000;  // 相手が防がなければ勝てる
  static constexpr int OPEN_THREE = 8'000;    // 次にOpenFourにできる (非常に強い)
  static constexpr int CLOSED_THREE = 500;    // 弱い攻撃
  static constexpr int OPEN_TWO = 100;        // 将来性

  static constexpr int CAPTURE_SCORE = 1'000;  // 石を1ペア取る価値

  // --- Move Ordering用 (Heuristic) ---
  // 高ければ高いほど先に探索される

  // 1. 決定的な手
  static constexpr int PRIORITY_WIN = 2'000'000;        // 5連を作る、または10個目の捕獲
  static constexpr int PRIORITY_BLOCK_WIN = 1'000'000;  // 相手の5連を防ぐ

  // 2. 非常に強い手
  static constexpr int PRIORITY_OPEN_FOUR = 500'000;     // 自分のOpen4を作る
  static constexpr int PRIORITY_BLOCK_OPEN_4 = 400'000;  // 相手のOpen4を防ぐ

  // 3. 強い手
  static constexpr int PRIORITY_OPEN_THREE = 100'000;   // 自分のOpen3を作る
  static constexpr int PRIORITY_CAPTURE = 80'000;       // 捕獲する
  static constexpr int PRIORITY_BLOCK_OPEN_3 = 70'000;  // 相手のOpen3を防ぐ
};

class Evaluator {
 public:
  // 盤面全体の静的評価（Minimaxの葉ノードで使用）
  static int evaluate(const Board& board, Color aiColor);

  // 手の生成時に、その手がどれくらい有望かを簡易評価する（Move Ordering用）
  static int evaluateMovePriority(const Board& board, int x, int y, Color color);

 private:
  // パターン認識ヘルパー
  static constexpr int SHIFT_H = 1;
  static constexpr int SHIFT_V = BOARD_WIDTH;
  static constexpr int SHIFT_D1 = BOARD_WIDTH + 1;
  static constexpr int SHIFT_D2 = BOARD_WIDTH - 1;
  static constexpr std::array<int, 4> ALL_DIRS = {SHIFT_H, SHIFT_V, SHIFT_D1, SHIFT_D2};

  // ビット演算による高速パターンカウント
  static int _CountPatterns(const BoardType& myBoard, const BoardType& empty);

  // 座標ベースのパターンチェック（Move Ordering用）
  static int _CheckLineScore(const Board& board, int x, int y, int dx, int dy, Color myColor,
                             Color oppColor);
};
