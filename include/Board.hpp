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
// Constants & Configuration
// ==========================================

// ==========================================
// Helper Structures
// ==========================================

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
};

// Information about a move, including captures
struct AIMoveRecord {
  int moveIndex;          // 打った手
  uint64_t prevHash;      // 手を打つ前のハッシュ値
  int prevBlackCaptures;  // 手を打つ前の黒の捕獲数
  int prevWhiteCaptures;  // 手を打つ前の白の捕獲数
  Color currentTurn;

  // 捕獲された石のインデックスを記録（最大でも8個程度なので固定長で十分）
  int capturedCount;
  std::array<int, 8> capturedIndices;
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
   * @param index The linear index of the move.
   * @return true if the move was valid and executed.
   *         false if the move was invalid (out of bounds, occupied, double three).
   */
  bool makeMove(int index);

  bool makeMoveAI(int index);

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

  void undoAI();

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
  const BoardType& getSentinelStones() const;

  // -- Computed Bitsets --
  BoardType getEmptyStones() const;
  BoardType getOccupiedStones() const;

  // -- Game State --
  Color getCurrentTurn() const;
  Color getNextTurn() const;
  Player getCurrentPlayer() const;
  int getBlackCaptures() const;
  int getWhiteCaptures() const;

  // -- Special Rule Flags --
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
  int8_t _getCaptureCount(Color color) const;

  // -- Rule Implementations --

  /**
   * Processes capture logic after a move at 'index'.
   * Updates capture counts and removes stones from bit boards.
   * Updates the _capturedStatus flag.
   * @param index The index of the newly placed stone.
   * @param record Optional AIMoveRecord to log captured stones. if nullptr, no logging is done.
   */
  void _processCapture(int index, AIMoveRecord* record);

  /**
   * Checks if the move at (x, y) creates a forbidden "Double Three".
   * A double-three is two simultaneous "open threes".
   * Updates the _doubleThreeStatus flag.
   * @param index The index of the newly placed stone.
   * @return true if the move creates a double three.
   */
  void _DoubleThree(int index);

  /**
   * Retrieves a 11-bit representation of stones along a line centered at (x, y).
   * The center bit (bit 5) corresponds to (x, y).
   * @param dir The direction to extract the line.
   * @return LineBits containing my and opponent stones along the line.
   */
  LineBits _getLineBits(int centerIndex, int offset, const BoardType& myStones,
                        const BoardType& oppStones) const;

  /**
   * Low-level check for a "Free Three" pattern in a specific direction.
   * @param line The 11-bit line representation centered at the move.
   */
  bool _checkFreeThree(LineBits line) const;

  /**
   * Checks if a detected 5-in-a-row line is safe from capture.
   * @param startIdx The starting index of the 5-in-a-row line.
   * @param shift The bitshift offset representing the line direction.
   * @return true if the line is safe (not capturable).
   */
  bool _isWinningLineSafe(int startIdx, int shift, const BoardType& myStones,
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
  BoardType _sentinelStones;  // Padding walls to simplify boundary checks

  // Game State
  Color _currentTurn;
  int8_t _blackCaptures;
  int8_t _whiteCaptures;

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
