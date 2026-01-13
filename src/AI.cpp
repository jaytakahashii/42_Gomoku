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

Move AI::getBestMove(const Board& board, Color color, AILevel level,
                     std::atomic<bool>& cancelFlag) {
  _aiPlayer = color;

  // Calculate target depth
  int targetDepth = _getDepthFromLevel(level);

  // Prepare variables for Iterative Deepening
  Move globalBestMove = {-1, -1, 0};

  // Clone the board ONCE for the search process
  Board searchBoard = board;

  // =========================================================
  // Iterative Deepening (ID)
  // Start from depth 2 and increase until targetDepth.
  // This fills the TT with good moves, making deeper searches faster.
  // =========================================================
  for (int depth = 2; depth <= targetDepth; ++depth) {
    // --- 1. Move Generation & Ordering ---
    // TT will now provide the "Best Move" from the previous depth (depth-1)
    // to sort moves efficiently.
    std::vector<Move> moves = _generateMoves(searchBoard, depth);

    if (moves.empty())
      return {-1, -1, 0};
    if (moves.size() == 1)
      return moves[0];  // Optimization

    // Hash Move Check (Check TT for the root position)
    uint64_t rootHash = searchBoard.getHash();
    TTEntry* entry = _tt.get(rootHash);

    if (entry != nullptr && entry->bestMove.x != -1) {
      // Move the best move from previous depth to the front
      for (size_t i = 0; i < moves.size(); ++i) {
        if (moves[i].x == entry->bestMove.x && moves[i].y == entry->bestMove.y) {
          std::swap(moves[0], moves[i]);
          break;
        }
      }
    }

    // --- 2. Root Search Loop ---
    Move currentDepthBestMove = moves[0];
    currentDepthBestMove.score = std::numeric_limits<int>::min();

    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    for (const Move& m : moves) {
      if (!searchBoard.makeMove(m.x, m.y))
        continue;

      // Win check optimization
      if (searchBoard.checkWin()) {
        searchBoard.undo();
        // Found a winning move at this depth.
        // Store and return immediately (no need to search deeper)
        _tt.store(rootHash, depth, ScoreConfig::WIN, TTFlag::EXACT, m);
        printPV(board);
        return m;
      }

      searchBoard.changeTurn();

      // Search children with reduced depth
      int score = _minimax(searchBoard, depth - 1, alpha, beta, false, cancelFlag);

      searchBoard.undo();

      // Update Best Move for this depth
      if (score > currentDepthBestMove.score) {
        currentDepthBestMove = m;
        currentDepthBestMove.score = score;
      }

      // Alpha Update
      if (currentDepthBestMove.score > alpha) {
        alpha = currentDepthBestMove.score;
      }

      // Win Threshold Break
      if (alpha >= ScoreConfig::WIN - 1000) {
        break;
      }
    }

    // --- 3. Update Global Best & Store to TT ---
    globalBestMove = currentDepthBestMove;

    // ★ CRITICAL: Store the Root Node result to TT
    // This allows the next iteration (depth+1) to use this result for sorting,
    // AND allows printPV to find the start of the chain.
    TTFlag flag =
        TTFlag::EXACT;  // Root node is usually exact unless alpha/beta cut logic is complex
    _tt.store(rootHash, depth, globalBestMove.score, flag, globalBestMove);

    // Debug output for each depth (Optional)
    // std::cout << "Depth " << depth << " done. Best: " << globalBestMove.x << "," <<
    // globalBestMove.y << std::endl;
  }

  // Print PV using the original board (hash matches rootHash)
  printPV(board);

  return globalBestMove;
}

int AI::_minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer,
                 std::atomic<bool>& cancelFlag) {
  // [1] TT Lookup (既存コード) ...
  uint64_t key = board.getHash();
  TTEntry* ttEntry = _tt.get(key);
  if (ttEntry != nullptr && ttEntry->depth >= depth) {
    if (ttEntry->flag == TTFlag::EXACT)
      return ttEntry->score;
    else if (ttEntry->flag == TTFlag::LOWERBOUND)
      alpha = std::max(alpha, ttEntry->score);
    else if (ttEntry->flag == TTFlag::UPPERBOUND)
      beta = std::min(beta, ttEntry->score);
    if (alpha >= beta)
      return ttEntry->score;
  }

  if (cancelFlag.load()) {
    return 0;
  }
  // [2] Base Case (既存コード) ...
  if (depth == 0)
    return Evaluator::evaluate(board, _aiPlayer);

  // [3] Move Generation (depthを渡すよう変更済み)
  std::vector<Move> moves = _generateMoves(board, depth);
  if (moves.empty())
    return 0;

  // TT Move Ordering (既存コード) ...
  if (ttEntry != nullptr && ttEntry->bestMove.x != -1) {
    // 先頭へスワップ
    for (size_t i = 0; i < moves.size(); ++i) {
      if (moves[i].x == ttEntry->bestMove.x && moves[i].y == ttEntry->bestMove.y) {
        std::swap(moves[0], moves[i]);
        break;
      }
    }
  }

  // --- PVS Search Loop ---

  int originalAlpha = alpha;
  Move bestMoveInThisNode = {-1, -1, 0};
  bool isFirstMove = true;  // ★ PVS用のフラグ

  if (maximizingPlayer) {
    int maxEval = std::numeric_limits<int>::min();

    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      // 即時勝利判定
      if (board.checkWin()) {
        board.undo();
        int winScore = ScoreConfig::WIN + depth;
        _tt.store(key, depth, winScore, TTFlag::EXACT, m);
        return winScore;
      }
      board.changeTurn();

      int eval;
      if (isFirstMove) {
        // 1. 最初の手（最善手候補）は全力で探索 (Full Window)
        eval = _minimax(board, depth - 1, alpha, beta, false, cancelFlag);
      } else {
        // 2. 2手目以降は Null Window Search (alpha, alpha+1)
        // 「今のalphaを超えないこと」を確認するだけの高速探索
        eval = _minimax(board, depth - 1, alpha, alpha + 1, false, cancelFlag);

        // もし alpha を超えていたら (Fail-High)、評価が間違っていた可能性があるので
        // 本来の窓 (alpha, beta) で再探索する
        if (eval > alpha && eval < beta) {
          eval = _minimax(board, depth - 1, alpha, beta, false, cancelFlag);
        }
      }

      board.undo();
      if (cancelFlag.load())
        return 0;

      if (eval > maxEval) {
        maxEval = eval;
        bestMoveInThisNode = m;
      }

      // Alpha Update
      alpha = std::max(alpha, maxEval);

      // Beta Cut-off
      if (beta <= alpha) {
        // ★ Killer Heuristic (Maximizerにとって、相手のこの分岐を断ち切る強い手)
        if (!(_killerMoves[depth][0].x == m.x && _killerMoves[depth][0].y == m.y)) {
          _killerMoves[depth][1] = _killerMoves[depth][0];
          _killerMoves[depth][0] = m;
        }
        break;
      }
      isFirstMove = false;  // 2周目からはfalse
    }
    // 結果保存 (既存コード)
    TTFlag flag = (maxEval <= originalAlpha)
                      ? TTFlag::UPPERBOUND
                      : (maxEval >= beta ? TTFlag::LOWERBOUND : TTFlag::EXACT);
    _tt.store(key, depth, maxEval, flag, bestMoveInThisNode);
    return maxEval;

  } else {
    // --- Minimizing Player (相手) ---
    int minEval = std::numeric_limits<int>::max();

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

      int eval;
      if (isFirstMove) {
        // 1. 最初の手は全力探索
        eval = _minimax(board, depth - 1, alpha, beta, true, cancelFlag);
      } else {
        // 2. 2手目以降は Null Window Search (beta-1, beta)
        // 「今のbetaを下回らないこと」を確認する
        eval = _minimax(board, depth - 1, beta - 1, beta, true, cancelFlag);

        // もし beta を下回っていたら (Fail-Low: 相手にとって良い手)、再探索
        if (eval < beta && eval > alpha) {
          eval = _minimax(board, depth - 1, alpha, beta, true, cancelFlag);
        }
      }

      board.undo();
      if (cancelFlag.load())
        return 0;

      if (eval < minEval) {
        minEval = eval;
        bestMoveInThisNode = m;
      }

      // Beta Update
      beta = std::min(beta, minEval);

      // Alpha Cut-off
      if (beta <= alpha) {
        // ★ Killer Heuristic (Minimizer分岐でのCut。必要ならここでも更新可)
        // 一般的にはMinimizer側でも有効な防御手などを登録する価値があります
        if (!(_killerMoves[depth][0].x == m.x && _killerMoves[depth][0].y == m.y)) {
          _killerMoves[depth][1] = _killerMoves[depth][0];
          _killerMoves[depth][0] = m;
        }
        break;
      }
      isFirstMove = false;
    }
    // 結果保存
    TTFlag flag = (minEval <= originalAlpha)
                      ? TTFlag::UPPERBOUND
                      : (minEval >= beta ? TTFlag::LOWERBOUND : TTFlag::EXACT);
    _tt.store(key, depth, minEval, flag, bestMoveInThisNode);
    return minEval;
  }
}

std::vector<Move> AI::_generateMoves(const Board& board, int depth) {
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

      // ★追加: キラー手ならボーナスを与える
      // (現在の深さがわからないので、引数に depth を渡すように変更する必要があります)
      // ここでは簡易的に「_generateMovesにdepthを渡す」修正が必要です。
      if (moves[i] == _killerMoves[depth][0])
        priority += 100000;  // Winよりは低いが非常に高く
      else if (moves[i] == _killerMoves[depth][1])
        priority += 90000;

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
    case AILevel::Normal:
      return DEPTH_NORMAL;
    case AILevel::Hard:
      return DEPTH_HARD;
    default:
      return DEPTH_HARD;
  }
}
