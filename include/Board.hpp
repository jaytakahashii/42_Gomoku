#pragma once

#include <array>
#include <bitset>
#include <map>
#include <vector>

#include "Enums.hpp"

// ==========================================
// Constants & Configuration
// ==========================================

// Board dimensions
// Using 32 as width for bit board efficiency (power of 2) and to provide
// sentinel padding preventing horizontal wrap-around during bitshifts.
constexpr int BOARD_SIZE = 19;
constexpr int BOARD_WIDTH = 32;
constexpr int MAX_CELLS = BOARD_WIDTH * BOARD_SIZE;

// Type alias for the bit board representation
using BoardType = std::bitset<MAX_CELLS>;

// ==========================================
// Helper Structures
// ==========================================

struct Direction {
  int dx;
  int dy;
};

// Snapshot of the board state for history/undo
struct BoardState {
  BoardType blackStones;
  BoardType whiteStones;
  int blackCaptures;
  int whiteCaptures;
  Color currentTurn;
};

// ==========================================
// Board Class
// ==========================================
class Board {
 public:
  // ----------------------------------------------------------------
  // Lifecycle & Setup
  // ----------------------------------------------------------------
  Board();
  ~Board() = default;

  /**
   * Assigns human/AI players to colors.
   */
  void setupPlayers(TurnOrder order);

  // ----------------------------------------------------------------
  // Core Gameplay Logic (Mutators)
  // ----------------------------------------------------------------

  /**
   * Attempts to place a stone at (x, y).
   * Handles rule validation, capture processing, and state updates.
   * @param x The x-coordinate (0-based).
   * @param y The y-coordinate (0-based).
   * @return true if the move was valid and executed.
   */
  bool makeMove(int x, int y);

  /**
   * Switches the current turn to the other player.
   */
  void changeTurn();

  /**
   * Saves the current board state to history for undo functionality.
   */
  void saveState();

  /**
   * Reverts the game to the previous state.
   * @return true if undo was successful (history not empty).
   */
  bool undo();

  // ----------------------------------------------------------------
  // Game Status & Win Conditions
  // ----------------------------------------------------------------

  /**
   * Checks if the game has reached a terminal state (Win by 5 or Capture).
   * @return true if the current player has won.
   */
  bool checkWin() const;

  /**
   * Checks if a specific color has won.
   * @param color The color to check for a win.
   * @return true if the specified color has won.
   */
  bool checkWin(Color color) const;

  // ----------------------------------------------------------------
  // State Queries (Getters)
  // ----------------------------------------------------------------

  // -- Board Information --
  Color getColorAt(int x, int y) const;
  Player getPlayerAt(int x, int y) const;

  // -- Stone Bitsets --
  const BoardType& getBlackStones() const;
  const BoardType& getWhiteStones() const;
  const BoardType& getMyStones(Color myColor) const;
  const BoardType& getOppStones(Color myColor) const;

  // -- Computed Bitsets --
  BoardType getEmptyStones() const;
  BoardType getOccupiedStones() const;

  // -- Game State --
  Color getCurrentTurn() const;
  Player getCurrentPlayer() const;
  int getBlackCaptures() const;
  int getWhiteCaptures() const;

  // -- Special Rule Flags --
  bool getDoubleThreeStatus() const;
  void setDoubleThreeStatus(bool status);
  bool getCapturedStatus() const;

  // -- AI Helpers --
  /**
   * Returns a bitset of stones that can be captured by myColor.
   * Useful for heuristic evaluation.
   * @param myColor The color of the player checking for capturable stones.
   * @return Bitset marking capturable stones.
   */
  BoardType getCapturableStones(Color myColor) const;

 private:
  // ----------------------------------------------------------------
  // Internal Helper Methods
  // ----------------------------------------------------------------

  // -- State Management --

  /**
   * Restores the board to a given state.
   */
  void _applyState(const BoardState& state);

  // -- Coordinate / Bit Utils --
  int _getIndex(int x, int y) const;

  // -- Rule Implementations --

  /**
   * Processes capture logic after a move at 'index'.
   * Updates capture counts and removes stones from bit boards.
   * Updates the _capturedStatus flag.
   * @param index The index of the newly placed stone.
   */
  void _processCapture(int index);

  /**
   * Checks if the move at (x, y) creates a forbidden "Double Three".
   * A double-three is two simultaneous "open threes".
   * Updates the _doubleThreeStatus flag.
   * @param x The x-coordinate of the move.
   * @param y The y-coordinate of the move.
   * @return true if the move creates a double three.
   */
  bool _isDoubleThree(int x, int y);

  /**
   * Low-level check for a "Free Three" pattern in a specific direction.
   */
  bool _checkFreeThree(int x, int y, int dir_x, int dir_y, const BoardType& myStones,
                       const BoardType& oppStones) const;

  /**
   * Checks if a stone at 'index' is currently vulnerable to capture.
   */
  bool _isStoneCapturable(int index, const BoardType& myStones, const BoardType& oppStones) const;

  /**
   * Returns a bit board marking positions that complete a 5-in-a-row.
   * @param shift_amount The bitshift offset representing a direction.
   */
  BoardType _getFiveInARowBits(const BoardType& stones, int shift_amount) const;

  // ----------------------------------------------------------------
  // Member Variables
  // ----------------------------------------------------------------

  // Board Data
  BoardType _blackStones;
  BoardType _whiteStones;

  // Game State
  Color _currentTurn;
  int8_t _blackCaptures;
  int8_t _whiteCaptures;

  // Flags for the last move (for UI or logic checks)
  bool _capturedStatus;
  bool _doubleThreeStatus;

  // Meta Data
  std::map<Color, Player> _colorToPlayer;
  std::vector<BoardState> _history;  // Stack for undo functionality

  // ----------------------------------------------------------------
  // Directional Constants
  // ----------------------------------------------------------------

  // Bitshift offsets for the 1D bitset (optimized for performance)
  static constexpr int SHIFT_H = 1;                 // Horizontal
  static constexpr int SHIFT_V = BOARD_WIDTH;       // Vertical
  static constexpr int SHIFT_D1 = BOARD_WIDTH + 1;  // Diagonal (Top-Left to Bottom-Right)
  static constexpr int SHIFT_D2 = BOARD_WIDTH - 1;  // Diagonal (Top-Right to Bottom-Left)

  static constexpr std::array<int, 4> ALL_SHIFTS = {SHIFT_H, SHIFT_V, SHIFT_D1, SHIFT_D2};

  // Coordinate deltas for 2D logic (e.g., checking surroundings)
  static constexpr std::array<Direction, 4> CHECK_DIRS = {{
      {1, 0},  // Horizontal
      {0, 1},  // Vertical
      {1, 1},  // Diagonal Down-Right
      {-1, 1}  // Diagonal Down-Left
  }};
};
