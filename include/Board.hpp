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
  bool checkWin(int x, int y);

 private:
  std::bitset<MAX_CELLS> _blackStones;
  std::bitset<MAX_CELLS> _whiteStones;
  Player _currentTurn;
  int getIndex(int x, int y) const;
};

#endif
