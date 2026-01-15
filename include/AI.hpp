#pragma once

#include <algorithm>
#include <atomic>
#include <limits>
#include <vector>

#include "Board.hpp"
#include "Enums.hpp"
#include "Evaluator.hpp"
#include "GameConfig.hpp"
#include "TranspositionTable.hpp"

// ==========================================
// AI Class
// ==========================================

class AI {
 public:
  AI();
  ~AI() = default;

  /**
   * Special handling for the second move under specific opening rules.
   * @param board The current board state.
   * @return The predetermined second move for the special rule, or {-1, 0} if not applicable.
   */
  Move getSecondMoveForSpecialRule(const Board& board);

  /**
   * Calculates the best move for the AI using Minimax with Alpha-Beta pruning.
   * @param board The current board state.
   * @param color The AI's color.
   * @param level The difficulty level of the AI.
   * @return The best move determined by the AI.
   */
  Move getBestMove(const Board& board, Color color, AILevel level, std::atomic<bool>& cancelFlag);

 private:
  // --- Configuration ---
  static constexpr int DEPTH_EASY = 2;
  static constexpr int DEPTH_NORMAL = 5;
  static constexpr int DEPTH_HARD = 10;

  static constexpr int MAX_MOVES_TO_CONSIDER = 14;
  static constexpr int SEARCH_WIDTH = 3;

  // --- Component State ---
  Color _aiPlayer;
  TranspositionTable _tt;
  Move _killerMoves[DEPTH_HARD][2];

  // --- Internal Logic ---

  /**
   * Minimax algorithm with Alpha-Beta pruning.
   * @param board The current board state (will be modified).
   * @param depth The remaining search depth.
   * @param alpha The Alpha value (best already explored option along the path to the root for the
   * maximizer).
   * @param beta The Beta value (best already explored option along the path to the root for the
   * minimizer).
   * @param maximizingPlayer true if the current player is the maximizer, false if minimizer.
   * @return The evaluated score for the current board state.
   */
  int _minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer,
               std::atomic<bool>& cancelFlag);

  /**
   * Generates a list of possible moves from the current board state.
   * @param board The current board state.
   * @return A vector of possible moves with heuristic scores. (MAX: MAX_MOVES_TO_CONSIDER)
   */
  std::vector<Move> _generateMoves(const Board& board, int depth,
                                   size_t limit = MAX_MOVES_TO_CONSIDER);

  std::vector<Move> _randomNeighbor(const BoardType& occupied);

  /**
   * Maps AI difficulty level to search depth.
   * @param level The AI difficulty level.
   * @return The corresponding search depth.
   */
  int _getDepthFromLevel(AILevel level);
};
