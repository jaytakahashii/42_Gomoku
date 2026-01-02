#include "AI.hpp"

Move AI::getBestMove(const Board& board, Color color, AILevel level) {
  Board clone = board;  // 盤面のコピー
  _aiPlayer = color;
  _startTime = std::chrono::high_resolution_clock::now();
  _timeOut = false;

  Move bestMove = {-1, -1, -std::numeric_limits<int>::max()};

  // 反復深化探索 (Iterative Deepening)
  for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
    int alpha = -std::numeric_limits<int>::max();
    int beta = std::numeric_limits<int>::max();

    // ルートでの手の生成
    std::vector<Move> moves = _generateMoves(clone);
    if (moves.empty())
      break;

    Move currentDepthBest = {-1, -1, -std::numeric_limits<int>::max()};
    bool timeExceededInLoop = false;

    // ルートノードのループ
    for (const Move& m : moves) {
      // 1. 手を打つ
      if (!clone.makeMove(m.x, m.y))
        continue;

      // 2. 探索
      int score = _minimax(clone, depth - 1, alpha, beta, false);

      // 3. 手を戻す
      clone.undo();

      if (_timeOut) {
        timeExceededInLoop = true;
        break;
      }

      if (score > currentDepthBest.score) {
        currentDepthBest = m;
      }

      alpha = std::max(alpha, score);
    }

    if (timeExceededInLoop) {
      std::cout << "Time up at depth " << depth << std::endl;
      break;
    }

    bestMove = currentDepthBest;
    std::cout << "Depth " << depth << " finished. Score: " << bestMove.score << std::endl;

    // 必勝が見えたら探索終了
    if (bestMove.score >= ScoreConfig::WIN - 1000)
      break;
  }

  return bestMove;
}

bool AI::_isTimeUp() {
  auto now = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime).count();
  return duration >= TIME_LIMIT_MS;
}

int AI::_minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
  // 定期的な時間チェック (探索回数ごとに毎回呼ぶと重いので、ビットマスク等で間引くのもあり)
  if (_timeOut || _isTimeUp()) {
    _timeOut = true;
    return 0;
  }

  // 1. 終局判定
  if (board.checkWin()) {
    // 深い階層（遠い未来）での勝ちは価値を少し下げる（最短勝ちを目指すため）
    return maximizingPlayer ? -ScoreConfig::WIN + depth : ScoreConfig::WIN - depth;
  }

  // 2. 葉ノード
  if (depth == 0) {
    return Evaluator::evaluate(board, _aiPlayer);
  }

  // 3. 手の生成
  std::vector<Move> moves = _generateMoves(board);
  if (moves.empty())
    return 0;  // 引き分け

  // 4. 再帰探索 (Undo利用)
  if (maximizingPlayer) {
    int maxEval = -std::numeric_limits<int>::max();
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      int eval = _minimax(board, depth - 1, alpha, beta, false);

      board.undo();  // 手を戻す

      if (_timeOut)
        return 0;

      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);
      if (beta <= alpha)
        break;  // Beta Cut-off
    }
    return maxEval;
  } else {
    int minEval = std::numeric_limits<int>::max();
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      int eval = _minimax(board, depth - 1, alpha, beta, true);

      board.undo();  // 手を戻す

      if (_timeOut)
        return 0;

      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);
      if (beta <= alpha)
        break;  // Alpha Cut-off
    }
    return minEval;
  }
}

std::vector<Move> AI::_generateMoves(const Board& board) {
  std::vector<Move> moves;
  BoardType visited;

  // 探索範囲を絞るための簡単なロジック
  // 石があるマスの近傍(radius=2)のみを候補とする
  bool isEmptyBoard = true;
  int size = BOARD_SIZE;
  int radius = 2;

  // 効率化: 本来はBoard側で「空きマスリスト」や「石のある場所リスト」を持つべきだが
  // ここではリファクタリング優先で元のロジックを踏襲しつつ整理
  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      if (board.getPlayerAt(x, y) != Player::NONE) {
        isEmptyBoard = false;

        for (int dy = -radius; dy <= radius; ++dy) {
          for (int dx = -radius; dx <= radius; ++dx) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx < 0 || nx >= size || ny < 0 || ny >= size)
              continue;
            if (board.getPlayerAt(nx, ny) != Player::NONE)
              continue;

            int idx = ny * BOARD_WIDTH + nx;
            if (!visited.test(idx)) {
              visited.set(idx);

              int priority = Evaluator::evaluateMovePriority(board, nx, ny, board.getCurrentTurn());
              moves.push_back({nx, ny, priority});
            }
          }
        }
      }
    }
  }

  if (isEmptyBoard) {
    moves.push_back({size / 2, size / 2, 0});
    return moves;
  }

  // Move Ordering: 優先度の高い順にソート
  // 安定ソートである必要はないのでstd::sortでOK
  // 探索後半での枝刈り効率に直結する重要な処理
  std::sort(moves.begin(), moves.end(),
            [](const Move& a, const Move& b) { return a.score > b.score; });

  // 上位N手のみ採用 (ビームサーチ的アプローチ)
  const size_t MAX_MOVES = 15;
  if (moves.size() > MAX_MOVES) {
    moves.resize(MAX_MOVES);
  }

  return moves;
}
