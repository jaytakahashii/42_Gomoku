#pragma once

#include <array>
#include <cmath>

#include "Board.hpp"

// ==========================================
// Score Configuration
// ==========================================
struct ScoreConfig {
  // --- Terminal States ---
  // Use a large value but allow space for depth bonuses (+ depth)
  static constexpr int WIN = 100'000'000;

  // --- Pattern Scores (Static Evaluation) ---
  // These values determine the AI's positional judgment.
  // Rule of thumb: Open N > Closed N > Open N-1

  static constexpr int FIVE = 1'000'000;      // 5 in a row (Immediate Win)
  static constexpr int OPEN_FOUR = 100'000;   // Unstoppable win next turn (.XXXX.)
  static constexpr int CLOSED_FOUR = 10'000;  // Forced defense required (X.XXX or .XXXXo)
  static constexpr int OPEN_THREE = 8'000;    // Major threat, creates Open Four (.XXX.)
  static constexpr int CLOSED_THREE = 500;    // Minor threat, enables expansion
  static constexpr int OPEN_TWO = 100;        // Good potential

  // --- Capture Incentives ---
  // Gomoku variant rules: Capturing is valuable, and 10 captures = Win.
  static constexpr int CAPTURE_SCORE = 2'000;       // Value of removing a pair
  static constexpr int CAPTURE_WIN_THRESHOLD = 10;  // Number of captures to win

  // --- Move Ordering Heuristics ---
  // Used solely for sorting moves in Alpha-Beta pruning.
  // Must be extremely fast to calculate.

  // Priority 1: Critical (Game Ending / Saving)
  static constexpr int PRIORITY_WIN = 2'000'000;
  static constexpr int PRIORITY_BLOCK_WIN = 1'000'000;  // Must block 5 or capture win

  // Priority 2: Major Threats
  static constexpr int PRIORITY_OPEN_FOUR = 500'000;
  static constexpr int PRIORITY_BLOCK_OPEN_4 = 400'000;

  // Priority 3: Offensive/Defensive Development
  static constexpr int PRIORITY_OPEN_THREE = 100'000;
  static constexpr int PRIORITY_CAPTURE = 40'000;        // Creating a capture
  static constexpr int PRIORITY_BLOCK_CAPTURE = 60'000;  // Saving own stones
  static constexpr int PRIORITY_BLOCK_OPEN_3 = 50'000;
};

// ==========================================
// Evaluator Class
// ==========================================

class Evaluator {
 public:
  /**
   * Performs a full static evaluation of the board.
   * This is slow and precise. Used only at leaf nodes (Depth 0).
   * @return Score from the perspective of aiColor (Positive = AI advantage).
   */
  static int evaluate(const Board& board, Color aiColor);

  /**
   * Quickly estimates the value of a single move at (x, y).
   * This is fast and approximate. Used for sorting moves (Move Ordering).
   * @return Raw priority score.
   */
  static int evaluateMovePriority(const Board& board, int index, Color color);

 private:
  // --- Helper Constants ---
  // Local copy of shift values to keep Evaluator self-contained
  static constexpr int SHIFT_H = 1;
  static constexpr int SHIFT_V = BOARD_WIDTH;
  static constexpr int SHIFT_D1 = BOARD_WIDTH + 1;
  static constexpr int SHIFT_D2 = BOARD_WIDTH - 1;
  static constexpr std::array<int, 4> ALL_DIRS = {SHIFT_H, SHIFT_V, SHIFT_D1, SHIFT_D2};

  // --- Internal Logic ---

  // Check line score based on coordinates (for Move Ordering)
  static int _CheckLineScore(int index, int offset, const BoardType& myStones,
                             const BoardType& oppStones, const BoardType& sentinels);

  // Evaluate score for a single color (used in full evaluation)
  static int _evaluateColor(const BoardType& stones, const BoardType& empty);
};
