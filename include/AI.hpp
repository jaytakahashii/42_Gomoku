#ifndef AI_HPP
#define AI_HPP

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "Board.hpp"
#include "Evaluator.hpp"

struct Move {
  int x;
  int y;
  int score;
};

class AI {
 public:
  AI() = default;

  // 制限時間内に最善手を計算して返す
  Move getBestMove(const Board& board, Color color, AILevel level);

 private:
  // --- 設定 ---
  static constexpr int TIME_LIMIT_MS = 450;
  static constexpr int MAX_DEPTH = 20;

  // --- 状態 ---
  Color _aiPlayer;
  bool _timeOut;
  std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;

  // --- 探索ロジック ---
  bool _isTimeUp();

  // Board& (参照) を受け取るように変更し、コピーを回避
  int _minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);

  // --- 手の生成 ---
  std::vector<Move> _generateMoves(const Board& board);
};

#endif
