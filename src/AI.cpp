#include "AI.hpp"

#include <iostream>

Move AI::getBestMove(const Board& board, Color color, AILevel level,
                     std::atomic<bool>& cancelFlag) {
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
    int score = _minimax(clone, maxDepth - 1, alpha, beta, false, cancelFlag);

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
int AI::_minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer,
                 std::atomic<bool>& cancelFlag) {
  if (cancelFlag.load()) {
    return 0;
  }
  // 1. Base Case: Leaf node reached
  if (depth == 0) {
    return Evaluator::evaluate(board, _aiPlayer);
  }

  // 2. Move Generation
  // Note: optimization (top 10 moves) is handled inside _generateMoves
  std::vector<Move> moves = _generateMoves(board);

  // Handle Draw/Stalemate
  if (moves.empty()) {
    return 0;
  }

  // 3. Recursive Search
  if (maximizingPlayer) {
    int maxEval = std::numeric_limits<int>::min();

    for (const Move& m : moves) {
      // Apply move
      if (!board.makeMove(m.x, m.y))
        continue;

      // Optimization: Check for immediate win BEFORE recursing
      // If this move wins, we don't need to look deeper.
      if (board.checkWin()) {
        board.undo();
        // Prefer winning sooner (higher depth remaining)
        return ScoreConfig::WIN + depth;
      }

      board.changeTurn();

      // Recurse
      int eval = _minimax(board, depth - 1, alpha, beta, false, cancelFlag);
      // Backtrack
      board.undo();
      if (cancelFlag.load())
        return 0;

      // Alpha-Beta Update
      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);

      // Beta Cut-off
      if (beta <= alpha) {
        break;
      }
    }
    return maxEval;

  } else {  // Minimizing Player (Opponent)
    int minEval = std::numeric_limits<int>::max();

    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      // Optimization: Check for immediate loss
      if (board.checkWin()) {
        board.undo();
        // Prefer losing later (lower depth remaining), result is negative
        return -(ScoreConfig::WIN + depth);
      }

      board.changeTurn();

      // Recurse
      int eval = _minimax(board, depth - 1, alpha, beta, true, cancelFlag);

      board.undo();
      if (cancelFlag.load())
        return 0;

      // Alpha-Beta Update
      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);

      // Alpha Cut-off
      if (beta <= alpha) {
        break;
      }
    }
    return minEval;
  }
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
