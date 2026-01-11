#pragma once

#include <limits>
#include <vector>

#include "Board.hpp"
#include "Enums.hpp"
#include "Evaluator.hpp"

// ==========================================
// Move Structure
// ==========================================

struct Move {
  int x;
  int y;
  int score;

  // Operator for sorting moves (descending order of score).
  // Crucial for Move Ordering in Alpha-Beta pruning.
  bool operator>(const Move& other) const {
    return score > other.score;
  }
};

// ==========================================
// AI Class
// ==========================================

class AI {
 public:
  AI() = default;
  ~AI() = default;

  /**
   * Calculates the best move for the AI using Minimax with Alpha-Beta pruning.
   * @param board The current board state.
   * @param color The AI's color.
   * @param level The difficulty level of the AI.
   * @return The best move determined by the AI.
   */
  Move getBestMove(const Board& board, Color color, AILevel level);

 private:
  // --- Configuration ---
  static constexpr int DEPTH_EASY = 5;
  static constexpr int DEPTH_NORMAL = 10;
  static constexpr int DEPTH_HARD = 15;

  static constexpr int MAX_MOVES_TO_CONSIDER = 10;

  // --- Component State ---
  Color _aiPlayer;

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
  int _minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);

  /**
   * Generates a list of possible moves from the current board state.
   * @param board The current board state.
   * @return A vector of possible moves with heuristic scores. (MAX: MAX_MOVES_TO_CONSIDER)
   */
  std::vector<Move> _generateMoves(const Board& board);

  /**
   * Maps AI difficulty level to search depth.
   * @param level The AI difficulty level.
   * @return The corresponding search depth.
   */
  int _getDepthFromLevel(AILevel level);
};
