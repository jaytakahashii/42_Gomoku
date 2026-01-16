#include "AI.hpp"

Move AI::getSecondMoveForSpecialRule(const Board& board) {
  BoardType occupied = board.getOccupiedStones();
  if (board.getOpeningRule() == OpeningRule::Pro) {
    if (occupied.test(PRO_CLOSEST_BOTTOM_INDEX)) {
      return {PRO_CLOSEST_TOP_INDEX, 0};
    } else {
      return {PRO_CLOSEST_BOTTOM_INDEX, 0};
    }
  } else if (board.getOpeningRule() == OpeningRule::LongPro) {
    if (occupied.test(LONG_PRO_CLOSEST_BOTTOM_INDEX)) {
      return {LONG_PRO_CLOSEST_TOP_INDEX, 0};
    } else {
      return {LONG_PRO_CLOSEST_BOTTOM_INDEX, 0};
    }
  }
  return {-1, 0};
}

Move AI::getBestMove(const Board& board, Color color, AILevel level,
                     std::atomic<bool>& cancelFlag) {
  _aiPlayer = color;
  int targetDepth = _getDepthFromLevel(level);
  Move globalBestMove = {-1, 0};
  Board searchBoard = board;

  // Iterative Deepening
  for (int depth = 2; depth <= targetDepth; ++depth) {
    // --- 1. Move Generation & Ordering ---
    std::vector<Move> moves = _generateMoves(searchBoard, depth, MAX_CELLS);

    if (moves.empty())
      return {-1, 0};
    if (moves.size() == 1)
      return moves[0];

    // Hash Move Check (Check TT for the root position)
    uint64_t rootHash = searchBoard.getHash();
    TTEntry* entry = _tt.get(rootHash);

    if (entry != nullptr && entry->bestMove.index != -1) {
      // Move the best move from previous depth to the front
      for (size_t i = 0; i < moves.size(); ++i) {
        if (moves[i].index == entry->bestMove.index) {
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

    for (size_t i = 0; i < moves.size(); ++i) {
      Move& m = moves[i];
      if (!searchBoard.makeMoveAI(m.index))
        continue;

      // Win check optimization
      if (searchBoard.checkWin()) {
        searchBoard.undoAI();
        _tt.store(rootHash, depth, ScoreConfig::WIN, TTFlag::EXACT, m);
        return m;
      }

      searchBoard.changeTurn();

      if (i >= MAX_MOVES_TO_CONSIDER) {
        searchBoard.undoAI();
        continue;
      }

      // Search children with reduced depth
      int score = _minimax(searchBoard, depth - 1, alpha, beta, false, cancelFlag);

      searchBoard.undoAI();

      // Update Best Move for this depth
      if (score > currentDepthBestMove.score) {
        currentDepthBestMove = m;
        currentDepthBestMove.score = score;
      }

      // Alpha Update
      if (currentDepthBestMove.score > alpha) {
        alpha = currentDepthBestMove.score;
      }

      if (cancelFlag.load())
        break;
    }

    // --- 3. Update Global Best & Store to TT ---
    if (!cancelFlag.load()) {
      globalBestMove = currentDepthBestMove;

      TTFlag flag = TTFlag::EXACT;
      _tt.store(rootHash, depth, globalBestMove.score, flag, globalBestMove);
    } else {
      break;
    }
  }

  return globalBestMove;
}

int AI::_minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer,
                 std::atomic<bool>& cancelFlag) {
  // [1] TT Lookup
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

  // [2] Base Case
  if (depth == 0)
    return Evaluator::evaluate(board, _aiPlayer);

  // [3] Move Generation
  std::vector<Move> moves = _generateMoves(board, depth, SEARCH_WIDTH);
  if (moves.empty())
    return 0;

  // TT Move Ordering
  if (ttEntry != nullptr && ttEntry->bestMove.index != -1) {
    for (size_t i = 0; i < moves.size(); ++i) {
      if (moves[i].index == ttEntry->bestMove.index) {
        std::swap(moves[0], moves[i]);
        break;
      }
    }
  }

  // --- PVS Search Loop ---

  int originalAlpha = alpha;
  Move bestMoveInThisNode = {-1, 0};
  bool isFirstMove = true;

  // --- Maximizing Player (AI) ---
  if (maximizingPlayer) {
    int maxEval = std::numeric_limits<int>::min();

    for (const Move& m : moves) {
      if (!board.makeMoveAI(m.index))
        continue;

      if (board.checkWin()) {
        board.undoAI();
        int winScore = ScoreConfig::WIN + depth;
        _tt.store(key, depth, winScore, TTFlag::EXACT, m);
        return winScore;
      }
      board.changeTurn();

      int eval;
      if (isFirstMove) {
        // 1. Full Window
        eval = _minimax(board, depth - 1, alpha, beta, false, cancelFlag);
      } else {
        // 2. Null Window Search (alpha, alpha+1)
        eval = _minimax(board, depth - 1, alpha, alpha + 1, false, cancelFlag);

        if (eval > alpha && eval < beta) {
          eval = _minimax(board, depth - 1, alpha, beta, false, cancelFlag);
        }
      }

      board.undoAI();
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
        if (_killerMoves[depth][0].index != m.index) {
          _killerMoves[depth][1] = _killerMoves[depth][0];
          _killerMoves[depth][0] = m;
        }
        break;
      }
      isFirstMove = false;
    }

    // Store Result
    TTFlag flag = (maxEval <= originalAlpha)
                      ? TTFlag::UPPERBOUND
                      : (maxEval >= beta ? TTFlag::LOWERBOUND : TTFlag::EXACT);
    _tt.store(key, depth, maxEval, flag, bestMoveInThisNode);
    return maxEval;

  } else {
    // --- Minimizing Player (Opp) ---
    int minEval = std::numeric_limits<int>::max();

    for (const Move& m : moves) {
      if (!board.makeMoveAI(m.index))
        continue;

      if (board.checkWin()) {
        board.undoAI();
        int loseScore = -(ScoreConfig::WIN + depth);
        _tt.store(key, depth, loseScore, TTFlag::EXACT, m);
        return loseScore;
      }
      board.changeTurn();

      int eval;
      if (isFirstMove) {
        eval = _minimax(board, depth - 1, alpha, beta, true, cancelFlag);
      } else {
        eval = _minimax(board, depth - 1, beta - 1, beta, true, cancelFlag);

        if (eval < beta && eval > alpha) {
          eval = _minimax(board, depth - 1, alpha, beta, true, cancelFlag);
        }
      }

      board.undoAI();
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
        if (_killerMoves[depth][0].index != m.index) {
          _killerMoves[depth][1] = _killerMoves[depth][0];
          _killerMoves[depth][0] = m;
        }
        break;
      }
      isFirstMove = false;
    }

    TTFlag flag = (minEval <= originalAlpha)
                      ? TTFlag::UPPERBOUND
                      : (minEval >= beta ? TTFlag::LOWERBOUND : TTFlag::EXACT);
    _tt.store(key, depth, minEval, flag, bestMoveInThisNode);
    return minEval;
  }
}

std::vector<Move> AI::_generateMoves(const Board& board, int depth, size_t limit) {
  std::vector<Move> moves;
  moves.reserve(BOARD_SIZE * BOARD_SIZE);

  BoardType occupied = board.getOccupiedStones();

  if (occupied.count() == 1) {
    return _randomNeighbor(occupied);
  }

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
      int priority = Evaluator::evaluateMovePriority(board, i, board.getCurrentTurn());

      // Killer Move Logic
      if (i == _killerMoves[depth][0].index)
        priority += 100000;
      else if (i == _killerMoves[depth][1].index)
        priority += 90000;

      moves.push_back({i, priority});
    }
  }

  // 4. Sort and Prune (Beam Search approach)
  if (moves.empty()) {
    return moves;
  }

  // Sort moves: Highest score first
  std::sort(moves.begin(), moves.end(), std::greater<Move>());

  if (moves.size() > limit) {
    moves.resize(limit);
  }

  return moves;
}

std::vector<Move> AI::_randomNeighbor(const BoardType& occupied) {
  std::vector<Move> moves;
  moves.reserve(1);

  // 1. Find the occupied stone
  // count() == 1 の前提で呼び出されるため、最初のビットが見つかればOKです
  int baseIndex = -1;
  for (int i = 0; i < MAX_CELLS; ++i) {
    if (occupied.test(i)) {
      baseIndex = i;
      break;
    }
  }

  if (baseIndex == -1) {
    return moves;
  }

  // 2. Define offsets for 8 directions
  // BOARD_WIDTH = 32
  // 上(-32), 下(+32), 左(-1), 右(+1), および斜め
  static const int offsets[8] = {
      -BOARD_WIDTH - 1,
      -BOARD_WIDTH,
      -BOARD_WIDTH + 1,  // Upper-Left, Up, Upper-Right
      -1,
      1,  // Left, Right
      BOARD_WIDTH - 1,
      BOARD_WIDTH,
      BOARD_WIDTH + 1  // Lower-Left, Down, Lower-Right
  };

  // Base X coordinate for wrapping check (0-31)
  int baseX = baseIndex & 31;  // equivalent to: baseIndex % 32

  // 3. Randomize search start direction
  // 元のコードは固定順序でしたが、AIの挙動としてランダムな方向から探す方が自然です
  int startDir = rand() % 8;

  for (int k = 0; k < 8; ++k) {
    // ランダムな位置から8方向を巡回
    int dirIdx = (startDir + k) % 8;
    int offset = offsets[dirIdx];

    int nIndex = baseIndex + offset;

    // [A] 配列の範囲チェック
    if (nIndex < 0 || nIndex >= MAX_CELLS)
      continue;

    // [B] 横方向のラップアラウンド（折り返し）チェック
    // 1次元配列上で単に -1 すると、行が変わって右端に行ってしまうのを防ぐ
    int nX = nIndex & 31;  // nIndex % 32
    if (std::abs(baseX - nX) > 1)
      continue;

    // [C] 盤面の有効範囲チェック
    // パディング領域(x >= 19)への着手を禁止
    if (nX >= BOARD_SIZE)
      continue;

    // [D] 空きマスかどうかチェック
    if (!occupied.test(nIndex)) {
      // 見つかったら即座に返す
      moves.push_back({nIndex, 0});
      return moves;
    }
  }

  return moves;  // 周囲がすべて埋まっている場合
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
