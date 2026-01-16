#pragma once

#include <array>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "Enums.hpp"
#include "GameConfig.hpp"
#include "Zobrist.hpp"

// ==========================================
// Helper Structures
// ==========================================

using BoardType = std::bitset<MAX_CELLS>;

struct Direction {
  int dx;
  int dy;
};

struct LineBits {
  uint16_t my;
  uint16_t opp;
};

// Snapshot of the board state for history/undo
struct BoardState {
  BoardType blackStones;
  BoardType whiteStones;
  int blackCaptures;
  int whiteCaptures;
  Color currentTurn;
  uint64_t hash;
  int handCount;
};

// Information about a move, including captures
struct AIMoveRecord {
  int moveIndex;
  uint64_t prevHash;
  int prevBlackCaptures;
  int prevWhiteCaptures;
  Color currentTurn;

  int capturedCount;
  std::array<int, 16> capturedIndices;
};

// ==========================================
// Board Class
// ==========================================

/**
 * @class Board
 * @brief Represents the game board and encapsulates all game logic.
 * Handles stone placements, captures, win conditions, and state management...
 */
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
   * Checks if the current player can make a move.
   * @return true if moves are possible, false if no valid moves remain.
   */
  bool canMove();

  /**
   * @brief Attempts to place a stone at the specified index.
   * @param index The linear index of the move.
   * @return true if the move was valid and executed.
   *         false if the move was invalid (out of bounds, occupied, double three).
   */
  bool makeMove(int index);

  /**
   * @brief AI makes a move at the specified index.
   * @param index The linear index of the move.
   * @return true if the move was valid and executed.
   *         false if the move was invalid.
   */
  bool makeMoveAI(int index);

  /**
   * @brief Switches the current turn to the other player.
   */
  void changeTurn();

  /**
   * @brief Saves the current board state to history for undo functionality.
   */
  void saveState();

  /**
   * @brief Reverts the game to the previous state.
   * @return true if undo was successful (history not empty).
   */
  bool undo();

  /**
   * @brief Reverts the last AI move, restoring captures and state.
   */
  void undoAI();

  // ----------------------------------------------------------------
  // Game Status & Win Conditions
  // ----------------------------------------------------------------

  /**
   * @brief Checks if the game has reached a terminal state (Win by 5 or Capture).
   * @return true if the current player has won.
   */
  bool checkWin() const;

  /**
   * @brief Checks if a specific color has won.
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
  const BoardType& getSentinelStones() const;

  // -- Computed Bitsets --
  BoardType getEmptyStones() const;
  BoardType getOccupiedStones() const;

  // -- Game State --
  int getHandCount() const;
  Color getCurrentTurn() const;
  Color getNextTurn() const;
  Player getCurrentPlayer() const;
  int getBlackCaptures() const;
  int getWhiteCaptures() const;
  bool isPrePlayerCannotMove() const;
  void setPrePlayerCannotMove(bool status);

  // -- Special Rule Flags / State --
  void setOpeningRule(OpeningRule rule);
  OpeningRule getOpeningRule() const;
  bool getDoubleThreeStatus() const;
  void setDoubleThreeStatus(bool status);
  bool getCapturedStatus() const;
  bool getForbiddenHandStatus() const;
  void setForbiddenHandStatus(bool status);

  // -- Hashing --
  uint64_t getHash() const;

  // -- Helpers --
  std::pair<int, int> getCoordinates(int index) const;
  int getIndex(int x, int y) const;

  // -- AI Helpers --
  /**
   * @brief Returns a bitset of stones that can be captured by myColor.
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
   * @brief Restores the board to a given state.
   */
  void _applyState(const BoardState& state);

  // -- Coordinate / Bit Utils --

  /**
   * @brief Retrieves the capture count for the specified color.
   */
  int8_t _getCaptureCount(Color color) const;

  // -- Rule Implementations --

  /**
   * @brief Processes captures resulting from the last move.
   * @param index The index of the newly placed stone.
   * @param record Optional AIMoveRecord to log captured stones. if nullptr, no logging is done.
   */
  void _processCapture(int index, AIMoveRecord* record);

  /**
   * @brief Checks and updates the double three status after a move.
   * @param index The index of the newly placed stone.
   * @return true if the move creates a double three.
   */
  void _DoubleThree(int index);

  /**
   * @brief Retrieves a 11-bit representation of stones along a line centered at (x, y).
   * @param dir The direction to extract the line.
   * @return LineBits containing my and opponent stones along the line.
   */
  LineBits _getLineBits(int centerIndex, int offset, const BoardType& myStones,
                        const BoardType& oppStones) const;

  /**
   * @brief Low-level check for a "Free Three" pattern in a specific direction.
   * @param line The 11-bit line representation centered at the move.
   */
  bool _checkFreeThree(LineBits line) const;

  /**
   * @brief Validates if a detected 5-in-a-row line is safe from capture.
   * @param startIdx The starting index of the 5-in-a-row line.
   * @param shift The bitshift offset representing the line direction.
   * @return true if the line is safe (not capturable).
   */
  bool _isWinningLineSafe(int startIdx, int shift, const BoardType& myStones,
                          const BoardType& oppStones) const;

  /**
   * @brief Checks if a stone at 'index' is currently vulnerable to capture.
   */
  bool _isStoneCapturable(int index, const BoardType& myStones, const BoardType& oppStones) const;

  /**
   * @brief Returns a bit board marking positions that complete a 5-in-a-row.
   * @param shift_amount The bitshift offset representing a direction.
   */
  BoardType _getFiveInARowBits(const BoardType& stones, int shift_amount) const;

  // ----------------------------------------------------------------
  // Member Variables
  // ----------------------------------------------------------------

  // Board Data
  BoardType _blackStones;
  BoardType _whiteStones;
  BoardType _sentinelStones;  // Padding walls to simplify boundary checks
  BoardType _forbiddenHandsOfPro;
  BoardType _forbiddenHandsOfLongPro;

  // Game State
  int _handCount;
  Color _currentTurn;
  int8_t _blackCaptures;
  int8_t _whiteCaptures;
  OpeningRule _openingRule;
  bool _prePlayerCannotMove;

  // Flags for the last move (for UI or logic checks)
  bool _capturedStatus;
  bool _doubleThreeStatus;
  bool _forbiddenHandStatus;

  // Meta Data
  std::map<Color, Player> _colorToPlayer;
  std::vector<BoardState> _history;      // For undo functionality
  std::vector<AIMoveRecord> _aiHistory;  // For AI move tracking

  // Hashing
  uint64_t _currentHash;

  // ----------------------------------------------------------------
  // Directional Constants
  // ----------------------------------------------------------------
  static const BoardType _validMask;
};
