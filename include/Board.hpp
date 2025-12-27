#ifndef BOARD_HPP
#define BOARD_HPP

#include <bitset>
#include <vector>

const unsigned int BOARD_SIZE = 19;
const int BOARD_WIDTH = 20;  // 19 + 1 sentinel
const int MAX_CELLS = BOARD_WIDTH * BOARD_SIZE;

enum class Player { NONE, BLACK, WHITE };

class Board {
 public:
  Board();

  bool makeMove(int x, int y);
  Player getStoneAt(int x, int y) const;
  Player getCurrentTurn() const;
  bool checkWin();
  void changeTurn();
  int getBlackCaptures() const;
  int getWhiteCaptures() const;

 private:
  using BoardType = std::bitset<MAX_CELLS>;

  BoardType _blackStones;
  BoardType _whiteStones;
  Player _currentTurn;
  int _blackCaptures;
  int _whiteCaptures;
  static constexpr int SHIFT_H = 1;                 // 横
  static constexpr int SHIFT_V = BOARD_WIDTH;       // 縦 (20)
  static constexpr int SHIFT_D1 = BOARD_WIDTH + 1;  // 右下 (21)
  static constexpr int SHIFT_D2 = BOARD_WIDTH - 1;  // 左下 (19)

  int _getIndex(int x, int y) const;
  bool _hasFiveInARow(const BoardType& stones, int shift_amount) const;
  void _checkAndProcessCapture(int index);
};

#endif
