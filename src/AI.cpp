#include "AI.hpp"

#include <iostream>

Move AI::getBestMove(const Board& board, Color color, AILevel level) {
  // 1. セットアップ
  Board clone = board;
  _aiPlayer = color;
  // int maxDepth = _getDepthFromLevel(level);
  int maxDepth = DEPTH_HARD;  // TODO: 一時的に固定

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

  Move bestMove = {-1, -1, -std::numeric_limits<int>::max()};
  int alpha = -std::numeric_limits<int>::max();
  int beta = std::numeric_limits<int>::max();

  // 3. ルートノード探索 (Minimaxの開始点)
  // 反復深化のループは削除し、いきなり maxDepth で探索します
  for (const Move& m : moves) {
    // 手を打つ
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

      // デバッグ表示（現在の一番良い手）
      // std::cout << "Candidate: (" << m.x << "," << m.y << ") Score: " << score << std::endl;
    }

    // Alpha値の更新
    alpha = std::max(alpha, score);

    // ルートノードでのBetaカット（理論上は発生しないが、必勝手が見つかったら打ち切るなど）
    if (score >= ScoreConfig::WIN - 1000) {
      break;  // 勝ち確定ならこれ以上探さない
    }
  }

  return bestMove;
}

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
    int maxEval = -std::numeric_limits<int>::max();
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

  // Boardクラスに追加した getEmptyStones() を活用
  BoardType empty = board.getEmptyStones();
  BoardType occupied = board.getOccupiedStones();

  if (occupied.none()) {
    moves.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2, 0});
    return moves;
  }

  // 石がある場所の周囲2マスを候補とする (ビット演算による高速化)
  BoardType candidates;
  const int W = BOARD_WIDTH;

  // シフト方向 (上下左右 + 斜め)
  const int shifts[] = {1, -1, W, -W, W + 1, -(W + 1), W - 1, -(W - 1)};

  BoardType mask = occupied;
  // Radius 1 & 2
  for (int s : shifts) {
    BoardType s1 = (s > 0) ? (occupied << s) : (occupied >> -s);
    BoardType s2 = (s > 0) ? (occupied << (s * 2)) : (occupied >> (-s * 2));
    mask |= s1 | s2;
  }

  // 「石の近く」かつ「空いている」場所
  candidates = mask & empty;

  // 候補をMoveリストに変換
  for (int i = 0; i < MAX_CELLS; ++i) {
    if (candidates.test(i)) {
      int y = i / W;
      int x = i % W;
      if (x >= BOARD_SIZE)
        continue;  // 番兵チェック

      // 優先度評価
      int priority = Evaluator::evaluateMovePriority(board, x, y, board.getCurrentTurn());
      moves.push_back({x, y, priority});
    }
  }

  // Move Ordering (非常に重要)
  // 良い手から先に探索することでAlpha-Beta枝刈りが効率的に働く
  if (moves.size() > 1) {
    std::sort(moves.begin(), moves.end(),
              [](const Move& a, const Move& b) { return a.score > b.score; });

    // 上位N手のみ採用 (Beam Search)
    // 深さ20を読むなら、ここの絞り込みは必須です
    if (moves.size() > 15) {
      moves.resize(15);
    }
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
