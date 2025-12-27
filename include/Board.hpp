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
  static constexpr std::array<int, 4> ALL_DIRS = {SHIFT_H, SHIFT_V, SHIFT_D1, SHIFT_D2};

  int _getIndex(int x, int y) const;
  void _checkAndProcessCapture(int index);
  BoardType _getFiveInARowBits(const BoardType& stones, int shift_amount) const;
  bool _isStoneCapturable(int index, const BoardType& myStones, const BoardType& oppStones) const;
};

#endif
