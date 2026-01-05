#include "AI.hpp"

#include <iostream>

/**
 * Arguments:
 * - board: 現在の盤面状態 (参照渡し)
 * - color: AIの色
 * - level: AIの難易度
 */
Move AI::getBestMove(const Board& board, Color color, AILevel level) {
  // 1. セットアップ
  Board clone = board;  // 盤面のクローンを作成 (1回のみ)
  _aiPlayer = color;
  int maxDepth = _getDepthFromLevel(level);

  std::cout << "AI Thinking... (Depth: " << maxDepth << ")" << std::endl;

  // 2. 手の生成（1回のみ実行）
  std::vector<Move> moves = _generateMoves(clone);

  // 打つ場所がない場合（引き分けや盤面埋まり）
  if (moves.empty()) {
    return {-1, -1, 0};
  }

  if (moves.size() == 1) {
    return moves[0];
  }

  Move bestMove = {-1, -1, std::numeric_limits<int>::min()};
  int alpha = std::numeric_limits<int>::min();
  int beta = std::numeric_limits<int>::max();

  // 3. ルートノード探索 (Minimaxの開始点)
  for (const Move& m : moves) {
    // 手を打つ (クローン上で)
    if (!clone.makeMove(m.x, m.y))
      continue;

    // 次は相手の番なので maximizingPlayer = false, 深さは -1
    int score = _minimax(clone, maxDepth - 1, alpha, beta, false);

    // 手を戻す
    clone.undo();

    // 最善手の更新
    if (score > bestMove.score) {
      bestMove = m;
      bestMove.score = score;
    }

    // Alpha値の更新
    alpha = std::max(alpha, score);
    std::cout << "alpha: " << alpha << ", beta: " << beta << std::endl;  // TODO: デバッグ用

    if (score >= ScoreConfig::WIN - 1000) {
      std::cout << "WINNNNN" << std::endl;  // TODO: デバッグ用
      break;                                // 勝ち確定ならこれ以上探さない
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
  // 1. 終局判定
  if (board.checkWin()) {
    // 自分が勝ったなら高得点。残り深さが大きい（早い勝ち）ほど高得点。
    return maximizingPlayer ? -(ScoreConfig::WIN + depth) : (ScoreConfig::WIN + depth);
  }

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

  std::sort(moves.begin(), moves.end(),
            [](const Move& a, const Move& b) { return a.score > b.score; });

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
