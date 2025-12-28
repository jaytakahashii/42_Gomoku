#ifndef AI_HPP
#define AI_HPP

#include <algorithm>
#include <limits>
#include <vector>

#include "Board.hpp"

// 点数定義
constexpr int SCORE_WIN = 1000000;
constexpr int SCORE_OPEN_FOUR = 100000;
constexpr int SCORE_CLOSED_FOUR = 10000;
constexpr int SCORE_OPEN_THREE = 5000;

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
  Player _aiPlayer;

  // Minimax法 (Alpha-Beta法)
  // board: 盤面のコピー (または参照+Undo)
  // depth: 残りの深さ
  // alpha, beta: 枝刈り用の値
  // maximizingPlayer: AIの手番ならtrue
  int _minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer);

  // 評価関数: 盤面の点数を計算する
  int _evaluate(const Board& board, Player player);

  // 有効な手（探索候補）を生成する
  // 全マス調べるのは遅いので、石の周囲だけを返すなどの工夫が必要
  std::vector<std::pair<int, int>> _generateMoves(const Board& board);

  // パターン評価用のヘルパー
  int _evaluateLine(const BoardType& myStones, const BoardType& oppStones) const;
};

#endif
