#ifndef AI_HPP
#define AI_HPP

#include <limits>
#include <vector>

#include "Board.hpp"
#include "Enums.hpp"
#include "Evaluator.hpp"

struct Move {
  int x;
  int y;
  int score;
};

class AI {
 public:
  AI() = default;

  // 指定された深さで最善手を計算して返す
  Move getBestMove(const Board& board, Color color, AILevel level);

 private:
  // --- 設定 ---

  // 各レベルの探索深さを定義
  static constexpr int DEPTH_EASY = 6;
  static constexpr int DEPTH_NORMAL = 10;
  static constexpr int DEPTH_HARD = 20;

  // --- 状態 ---
  Color _aiPlayer;

  // --- 探索ロジック ---
  // Minimax法（Alpha-Beta法）
  int _minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);

  // --- 手の生成 ---
  std::vector<Move> _generateMoves(const Board& board);

  // レベルから深さを取得するヘルパー
  int _getDepthFromLevel(AILevel level);
};

#endif
