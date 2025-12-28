#include "AI.hpp"

#include <iostream>

Move AI::getBestMove(Board board, Player player) {
  _aiPlayer = player;

  // 本来は Iterative Deepening (深さ1, 2, 3...) を行うが、
  // まずは固定深さでテスト
  int depth = 3;

  Move bestMove = {-1, -1, -std::numeric_limits<int>::max()};

  // 候補手を取得
  std::vector<std::pair<int, int>> moves = _generateMoves(board);

  for (const std::pair<int, int>& p : moves) {
    int x = p.first;
    int y = p.second;

    // 1手進める (コピーを作成してシミュレーション)
    Board nextBoard = board;
    if (!nextBoard.makeMove(x, y))
      continue;

    // Minimax呼び出し (次は相手の番なので maximizing=false)
    int score = _minimax(nextBoard, depth - 1, -std::numeric_limits<int>::max(),
                         std::numeric_limits<int>::max(), false);

    if (score > bestMove.score) {
      bestMove.x = x;
      bestMove.y = y;
      bestMove.score = score;
    }
  }

  return bestMove;
}

int AI::_minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer) {
  // 1. 終局判定 or 深さ制限到達
  if (board.checkWin()) {
    // 勝ったプレイヤーがAIなら高得点、敵なら低得点
    // 深さが浅い(早い)勝ちほど価値が高いように depth を加算する
    return maximizingPlayer ? -SCORE_WIN + depth : SCORE_WIN - depth;
  }
  if (depth == 0) {
    return _evaluate(board, _aiPlayer);
  }

  auto moves = _generateMoves(board);

  if (maximizingPlayer) {
    int maxEval = -std::numeric_limits<int>::max();
    for (const auto& p : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(p.first, p.second))
        continue;

      int eval = _minimax(nextBoard, depth - 1, alpha, beta, false);
      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);
      if (beta <= alpha)
        break;  // Beta cut-off
    }
    return maxEval;
  } else {
    int minEval = std::numeric_limits<int>::max();
    for (const auto& p : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(p.first, p.second))
        continue;

      int eval = _minimax(nextBoard, depth - 1, alpha, beta, true);
      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);
      if (beta <= alpha)
        break;  // Alpha cut-off
    }
    return minEval;
  }
}

// 非常に単純な候補手生成
// (最適化のためには、石があるマスの周囲2マス以内のみを返すようにする)
std::vector<std::pair<int, int>> AI::_generateMoves(const Board& board) {
  std::vector<std::pair<int, int>> moves;

  // 中心から探索するように並び替えるとAlpha-Beta剪定が効きやすい
  // とりあえず今は単純な全探索（遅いので後で最適化必須）
  for (int y = 0; y < BOARD_SIZE; ++y) {
    for (int x = 0; x < BOARD_SIZE; ++x) {
      if (board.getStoneAt(x, y) == Player::NONE) {
        // 石の周囲だけ探索する最適化を入れる場所
        // ここでは単純に全部追加
        moves.push_back({x, y});
      }
    }
  }
  return moves;
}

// 評価関数 (仮)
// 今は「自分の石の数」を返すだけなどの超適当な実装でOK
// 次のステップでここを強化します
int AI::_evaluate(const Board& board, Player player) {
  // TODO: パターンマッチングによる本格的な評価

  // 仮実装: ランダムに動かないように、中心に近いほど高得点にする
  int score = 0;
  // ...
  return score;
}
