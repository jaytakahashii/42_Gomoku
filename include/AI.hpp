#ifndef AI_HPP
#define AI_HPP

#include <algorithm>
#include <chrono>
#include <limits>
#include <vector>

#include "Board.hpp"

struct Move {
  int x;
  int y;
  int score;
};

class AI {
 public:
  AI() = default;

  // メイン関数: 制限時間内にベストな手を返す
  Move getBestMove(Board board, Player player);

 private:
  // 点数定義
  static constexpr int _SCORE_WIN = 100000000;
  static constexpr int _SCORE_OPEN_FOUR = 10000000;
  static constexpr int _SCORE_CLOSED_FOUR = 100000;
  static constexpr int _SCORE_OPEN_THREE = 100000;
  static constexpr int _SCORE_CAPTURE = 1000000;

  // 探索開始時刻
  std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
  // タイムアウトしたかどうかのフラグ
  bool _timeOut;

  // 時間チェック用関数
  bool _isTimeUp();

  Player _aiPlayer;

  // Minimax法 (Alpha-Beta法)
  // board: 盤面のコピー (または参照+Undo)
  // depth: 残りの深さ
  // alpha, beta: 枝刈り用の値
  // maximizingPlayer: AIの手番ならtrue
  int _minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer);

  // 評価関数: 盤面の点数を計算する
  int _evaluate(const Board& board, Player player);

  // 特定のパターンが盤面にいくつあるか数える高速関数
  // stones: 自分の石, empty: 空点
  int _countPatterns(const BoardType& stones, const BoardType& empty);

  // 有効な手（探索候補）を生成する
  // 全マス調べるのは遅いので、石の周囲だけを返すなどの工夫が必要
  std::vector<Move> _generateMoves(const Board& board);

  int _evaluateMoveOrdering(const Board& board, int x, int y, Player player);
  int _evaluatePoint(const Board& board, int x, int y, Player player);

  // パターン評価用のヘルパー
  int _evaluateLine(const BoardType& myStones, const BoardType& oppStones) const;
};

#endif
