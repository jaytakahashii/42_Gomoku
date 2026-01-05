#ifndef BOARD_HPP
#define BOARD_HPP

#include <Enums.hpp>
#include <array>
#include <bitset>
#include <map>
#include <vector>

#include "Enums.hpp"

constexpr int BOARD_SIZE = 19;
constexpr int BOARD_WIDTH = 32;  // 19 + sentinel
constexpr int MAX_CELLS = BOARD_WIDTH * BOARD_SIZE;

using BoardType = std::bitset<MAX_CELLS>;

struct Direction {
  int dx;
  int dy;
};

struct BoardState {
  BoardType blackStones;
  BoardType whiteStones;
  int blackCaptures;
  int whiteCaptures;
  Color currentTurn;
};

class Board {
 public:
  Board();

  // ゲーム進行用
  bool makeMove(int x, int y);
  void changeTurn();
  bool checkWin();
  bool checkWinWithFive() const;

  // 状態取得用
  Color getColorAt(int x, int y) const;
  Player getPlayerAt(int x, int y) const;
  Color getCurrentTurn() const;
  Player getCurrentPlayer() const;
  int getBlackCaptures() const;
  int getWhiteCaptures() const;
  bool getDoubleThreeStatus() const;
  bool getCapturedStatus() const;
  const BoardType& getBlackStones() const;
  const BoardType& getWhiteStones() const;
  const BoardType& getMyStones(Color myColor) const;
  const BoardType& getOppStones(Color myColor) const;
  BoardType getEmptyStones() const;
  BoardType getOccupiedStones() const;
  void setDoubleThreeStatus(bool status);
  void setupPlayers(TurnOrder order);

  void saveState();
  bool undo();

 private:
  BoardType _blackStones;
  BoardType _whiteStones;
  Color _currentTurn;
  int _blackCaptures;
  int _whiteCaptures;
  bool _doubleThreeStatus;
  bool _capturedStatus;

  std::map<Color, Player> _colorToPlayer;
  std::vector<BoardState> _history;

  // 方向定数
  static constexpr int SHIFT_H = 1;                 // 横
  static constexpr int SHIFT_V = BOARD_WIDTH;       // 縦 (20)
  static constexpr int SHIFT_D1 = BOARD_WIDTH + 1;  // 右下 (21)
  static constexpr int SHIFT_D2 = BOARD_WIDTH - 1;  // 左下 (19)
  static constexpr std::array<int, 4> ALL_DIRS = {SHIFT_H, SHIFT_V, SHIFT_D1, SHIFT_D2};

  static constexpr std::array<Direction, 4> CHECK_DIRS = {{
      {1, 0},  // Horizontal
      {0, 1},  // Vertical
      {1, 1},  // Diagonal Down-Right
      {-1, 1}  // Diagonal Down-Left
  }};

  int _getIndex(int x, int y) const;
  bool _checkAndProcessCapture(int index);
  BoardType _getFiveInARowBits(const BoardType& stones, int shift_amount) const;
  bool _isStoneCapturable(int index, const BoardType& myStones, const BoardType& oppStones) const;
  bool _checkFreeThree(int x, int y, int dir_x, int dir_y, const BoardType& myStones,
                       const BoardType& oppStones) const;
  bool _isDoubleThree(int x, int y);
  void _applyState(BoardState state);
};

#endif
