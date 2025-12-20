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

 private:
  std::bitset<MAX_CELLS> _blackStones;
  std::bitset<MAX_CELLS> _whiteStones;
  Player _currentTurn;
  static constexpr int SHIFT_H = 1;                 // 横
  static constexpr int SHIFT_V = BOARD_WIDTH;       // 縦 (20)
  static constexpr int SHIFT_D1 = BOARD_WIDTH + 1;  // 右下 (21)
  static constexpr int SHIFT_D2 = BOARD_WIDTH - 1;  // 左下 (19)

  int _getIndex(int x, int y) const;
  bool _hasFiveInARow(const std::bitset<MAX_CELLS>& stones, int shift_amount) const;
};

#endif
