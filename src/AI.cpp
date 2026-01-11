#include "AI.hpp"

#include <iostream>

Move AI::getBestMove(const Board& board, Color color, AILevel level) {
  // 1. Setup Phase
  Board clone = board;
  _aiPlayer = color;
  int maxDepth = _getDepthFromLevel(level);

  // 2. Move Generation & Ordering
  // Optimization: Generating moves on the cloned board.
  std::vector<Move> moves = _generateMoves(clone);

  // Edge Cases: No moves available or only one move
  if (moves.empty()) {
    return {-1, -1, 0};
  }
  if (moves.size() == 1) {
    return moves[0];
  }

  // 3. Root Search Initialization
  Move bestMove = moves[0];
  bestMove.score = std::numeric_limits<int>::min();

  int alpha = std::numeric_limits<int>::min();
  int beta = std::numeric_limits<int>::max();

  // 4. Root Loop (The first level of Minimax)
  for (const Move& m : moves) {
    // Execute move on the clone
    if (!clone.makeMove(m.x, m.y)) {
      continue;
    }

    // Note: Assuming makeMove does NOT change turn automatically.
    // If your Board::makeMove handles turn switching, remove this line.
    clone.changeTurn();

    // Recursive call: Next is opponent's turn (Minimizer)
    int score = _minimax(clone, maxDepth - 1, alpha, beta, false);

    // Undo move to restore state
    clone.undo();

    // Update Best Move (Maximizing at root)
    if (score > bestMove.score) {
      bestMove = m;
      bestMove.score = score;
    }

    // Alpha Update
    if (bestMove.score > alpha) {
      alpha = bestMove.score;
    }

    // Optimization: Early Exit on Victory
    // If we found a move that guarantees a win, we don't need to search further.
    if (alpha >= ScoreConfig::WIN - 1000) {
      return bestMove;
    }
  }

  return bestMove;
}

/**
 * Arguments:
 * - board: 現在の盤面状態（変更されるのでコピーを渡すこと）
 * - depth: 残りの探索深さ
 * - alpha: Alpha値（最良の選択肢の下限）
 * - beta: Beta値（最良の選択肢の上限）
 * - maximizingPlayer: 現在のプレイヤーが最大化を目指しているかどうか
 */
int AI::_minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
  // 2. 葉ノード（指定深さに到達）
  if (depth == 0) {
    return Evaluator::evaluate(board, _aiPlayer);
  }

  // 3. 手の生成
  std::vector<Move> moves = _generateMoves(board);
  if (moves.empty())
    return 0;  // 引き分け

  // 4. 再帰探索
  if (maximizingPlayer) {
    int maxEval = std::numeric_limits<int>::min();
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      // 終局判定
      if (board.checkWin()) {
        board.undo();
        return ScoreConfig::WIN + depth;  // 早く勝つほど高得点
      }

      board.changeTurn();
      int eval = _minimax(board, depth - 1, alpha, beta, false);

      board.undo();

      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);

      // Beta Cut-off
      if (beta <= alpha)
        break;
    }
    return maxEval;
  } else {
    int minEval = std::numeric_limits<int>::max();
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      // 終局判定
      if (board.checkWin()) {
        board.undo();
        return -(ScoreConfig::WIN + depth);  // 早く負けるほど低得点
      }

      board.changeTurn();
      int eval = _minimax(board, depth - 1, alpha, beta, true);

      board.undo();

      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);

      // Alpha Cut-off
      if (beta <= alpha)
        break;
    }
    return minEval;
  }
}

std::vector<Move> AI::_generateMoves(const Board& board) {
  std::vector<Move> moves;

  BoardType empty = board.getEmptyStones();
  BoardType occupied = board.getOccupiedStones();

  // 1. 初手（中央）
  if (occupied.none()) {
    moves.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2, 0});
    return moves;
  }

  // 2. 探索範囲の決定（石の周囲1マス）
  BoardType candidates;
  const int W = BOARD_WIDTH;
  const int shifts[] = {1, -1, W, -W, W + 1, -(W + 1), W - 1, -(W - 1)};

  BoardType mask = occupied;
  for (int s : shifts) {
    BoardType s1 = (s > 0) ? (occupied << s) : (occupied >> -s);
    mask |= s1;
  }
  candidates = mask & empty;

  for (int i = 0; i < MAX_CELLS; ++i) {
    if (candidates.test(i)) {
      int y = i / W;
      int x = i % W;
      if (x >= BOARD_SIZE)
        continue;

      int priority = Evaluator::evaluateMovePriority(board, x, y, board.getCurrentTurn());

      moves.push_back({x, y, priority});
    }
  }

  // 4. ソート (スコアが高い順)
  if (moves.empty())
    return moves;

  std::sort(moves.begin(), moves.end(), std::greater<Move>());
  if (moves.size() > MAX_MOVES_TO_CONSIDER) {
    moves.resize(MAX_MOVES_TO_CONSIDER);
  }

  return moves;
}

int AI::_getDepthFromLevel(AILevel level) {
  switch (level) {
    case AILevel::Easy:
      return DEPTH_EASY;
    case AILevel::Medium:
      return DEPTH_NORMAL;
    case AILevel::Hard:
      return DEPTH_HARD;
    default:
      return DEPTH_HARD;
  }
}
