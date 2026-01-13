#include "AI.hpp"

#include <iostream>

AI::AI() : _tt(20) {
}

// TODO: debug
// 現在の盤面から、TTを辿ってAIが考えている最善手順を表示する
void AI::printPV(Board board) {
  std::cout << "PV: ";
  for (int i = 0; i < 20; ++i) {
    uint64_t key = board.getHash();
    TTEntry* entry = _tt.get(key);

    // エントリがない、または最善手が記録されていないなら終了
    if (entry == nullptr || entry->bestMove.x == -1) {
      break;
    }

    Move m = entry->bestMove;
    std::cout << "(" << m.x << "," << m.y << ") -> ";

    if (!board.makeMove(m.x, m.y))
      break;
    if (board.checkWin()) {
      std::cout << "WIN";
      break;
    }
    board.changeTurn();
  }
  std::cout << std::endl;
}

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
      printPV(board);  // TODO: debug
      return bestMove;
    }
  }

  // TODO: debug
  printPV(board);

  return bestMove;
}

int AI::_minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
  // [1] Transposition Table Lookup (キャッシュ確認)
  // ---------------------------------------------------------
  uint64_t key = board.getHash();
  TTEntry* ttEntry = _tt.get(key);

  if (ttEntry != nullptr && ttEntry->depth >= depth) {
    // キャッシュされた結果が、今の探索よりも深い（＝信頼できる）場合
    if (ttEntry->flag == TTFlag::EXACT) {
      return ttEntry->score;
    } else if (ttEntry->flag == TTFlag::LOWERBOUND) {
      alpha = std::max(alpha, ttEntry->score);
    } else if (ttEntry->flag == TTFlag::UPPERBOUND) {
      beta = std::min(beta, ttEntry->score);
    }

    // キャッシュによって探索範囲が矛盾した（＝枝刈り可能）
    if (alpha >= beta) {
      return ttEntry->score;
    }
  }
  // ---------------------------------------------------------

  // [2] 終了条件
  if (depth == 0) {
    return Evaluator::evaluate(board, _aiPlayer);
  }

  // [3] 手の生成
  std::vector<Move> moves = _generateMoves(board);
  if (moves.empty())
    return 0;

  // [★重要] Hash Move Ordering
  // TTに「以前見つけた最善手(Best Move)」があれば、それを最優先で探索する。
  // これにより枝刈り効率が劇的に向上する。
  if (ttEntry != nullptr && ttEntry->bestMove.x != -1) {
    for (size_t i = 0; i < moves.size(); ++i) {
      if (moves[i].x == ttEntry->bestMove.x && moves[i].y == ttEntry->bestMove.y) {
        // 先頭の手と交換
        std::swap(moves[0], moves[i]);
        // moves[0].score = ...; // 必要ならスコアを最大にしておく
        break;
      }
    }
  }

  // --- 以降は通常のMinimaxループ ---

  int originalAlpha = alpha;  // フラグ判定用に保存
  Move bestMoveInThisNode = {-1, -1, 0};
  int bestScore =
      maximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

  if (maximizingPlayer) {
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      // 即時勝利判定
      if (board.checkWin()) {
        board.undo();
        int winScore = ScoreConfig::WIN + depth;  // 早く勝つ方が良い
        // 勝利確定も保存してリターン
        _tt.store(key, depth, winScore, TTFlag::EXACT, m);
        return winScore;
      }

      board.changeTurn();
      int score = _minimax(board, depth - 1, alpha, beta, false);
      board.undo();

      if (score > bestScore) {
        bestScore = score;
        bestMoveInThisNode = m;
      }
      alpha = std::max(alpha, bestScore);
      if (beta <= alpha)
        break;  // Beta Cut
    }
  } else {
    // Minimizing logic (対称の実装)
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      if (board.checkWin()) {
        board.undo();
        int loseScore = -(ScoreConfig::WIN + depth);
        _tt.store(key, depth, loseScore, TTFlag::EXACT, m);
        return loseScore;
      }

      board.changeTurn();
      int score = _minimax(board, depth - 1, alpha, beta, true);
      board.undo();

      if (score < bestScore) {
        bestScore = score;
        bestMoveInThisNode = m;
      }
      beta = std::min(beta, bestScore);
      if (beta <= alpha)
        break;  // Alpha Cut
    }
  }

  // [4] 結果の保存
  // ---------------------------------------------------------
  TTFlag flag;
  if (bestScore <= originalAlpha) {
    flag = TTFlag::UPPERBOUND;  // Fail-Low (Alpha Cut-offされなかったけど全部ダメ)
  } else if (bestScore >= beta) {
    flag = TTFlag::LOWERBOUND;  // Fail-High (Beta Cut-offされた)
  } else {
    flag = TTFlag::EXACT;  // Alpha < Score < Beta (正確な値)
  }

  _tt.store(key, depth, bestScore, flag, bestMoveInThisNode);
  // ---------------------------------------------------------

  return bestScore;
}

std::vector<Move> AI::_generateMoves(const Board& board) {
  std::vector<Move> moves;
  moves.reserve(64);  // Reserve memory to prevent reallocations

  BoardType occupied = board.getOccupiedStones();

  // 1. First Move Strategy (Center)
  // If the board is empty, always play the center (standard Gomoku strategy).
  if (occupied.none()) {
    moves.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2, 0});
    return moves;
  }

  if (occupied.count() == 1) {
    return _randomNeighbor(occupied);
  }

  // 2. Determine Search Scope (Radius 1 around existing stones)
  // Instead of a loop with branches, we apply bitwise operations directly.
  // This creates a mask of all cells adjacent to any stone.
  BoardType neighborMask = occupied;

  // Horizontal
  neighborMask |= (occupied << 1);
  neighborMask |= (occupied >> 1);
  // Vertical
  neighborMask |= (occupied << BOARD_WIDTH);
  neighborMask |= (occupied >> BOARD_WIDTH);
  // Diagonal
  neighborMask |= (occupied << (BOARD_WIDTH + 1));
  neighborMask |= (occupied >> (BOARD_WIDTH + 1));
  neighborMask |= (occupied << (BOARD_WIDTH - 1));
  neighborMask |= (occupied >> (BOARD_WIDTH - 1));

  // Filter: We only want cells that are currently empty
  // (board.getEmptyStones() already handles the valid board boundaries)
  BoardType candidates = neighborMask & board.getEmptyStones();

  // 3. Evaluate and Collect Candidates
  for (int i = 0; i < MAX_CELLS; ++i) {
    if (candidates.test(i)) {
      int x = i % BOARD_WIDTH;
      int y = i / BOARD_WIDTH;

      // Note: Since getEmptyStones() masks out the padding,
      // x will never be >= BOARD_SIZE. But a safety check is fine.

      // Calculate heuristic score for sorting
      // (This function must be lightweight!)
      int priority = Evaluator::evaluateMovePriority(board, x, y, board.getCurrentTurn());

      moves.push_back({x, y, priority});
    }
  }

  // 4. Sort and Prune (Beam Search approach)
  if (moves.empty()) {
    return moves;
  }

  // Sort moves: Highest score first
  std::sort(moves.begin(), moves.end(), std::greater<Move>());

  // Pruning: Only keep the top N moves to reduce search space.
  // WARNING: 'MAX_MOVES_TO_CONSIDER' (10) might be too aggressive.
  // If the opponent has a threat at the 11th best move, you will lose instantly.
  // Consider increasing this to 20-30 or using a dynamic threshold
  // (e.g., keep all moves within 500 points of the best move).
  if (moves.size() > MAX_MOVES_TO_CONSIDER) {
    moves.resize(MAX_MOVES_TO_CONSIDER);
  }

  return moves;
}

// return: 1 size list of random neighboring moves
std::vector<Move> AI::_randomNeighbor(const BoardType& occupied) {
  std::vector<Move> moves;
  moves.reserve(1);

  // Find all occupied positions
  std::vector<int> occupiedIndices;
  for (int i = 0; i < MAX_CELLS; ++i) {
    if (occupied.test(i)) {
      occupiedIndices.push_back(i);
      break;
    }
  }

  if (occupiedIndices.empty()) {
    return moves;  // No occupied stones, should not happen here
  }

  // Randomly select one occupied stone
  int randIndex = rand() % occupiedIndices.size();
  int baseIndex = occupiedIndices[randIndex];
  int baseX = baseIndex % BOARD_WIDTH;
  int baseY = baseIndex / BOARD_WIDTH;

  // Check neighboring cells (8 directions)
  std::vector<std::pair<int, int>> directions = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0},
                                                 {1, 0},   {-1, 1}, {0, 1},  {1, 1}};

  for (const auto& dir : directions) {
    int nx = baseX + dir.first;
    int ny = baseY + dir.second;

    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
      int nIndex = ny * BOARD_WIDTH + nx;
      if (!occupied.test(nIndex)) {
        moves.push_back({nx, ny, 0});
        return moves;  // Return immediately after finding the first valid neighbor
      }
    }
  }

  return moves;  // Fallback: no valid neighbors found
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
