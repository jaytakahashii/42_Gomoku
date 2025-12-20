#include <Board.hpp>

Board::Board() : _currentTurn(Player::BLACK) {
  _blackStones.reset();
  _whiteStones.reset();
}

bool Board::makeMove(int x, int y) {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    return false;

  int index = _getIndex(x, y);

  if (_blackStones.test(index) || _whiteStones.test(index)) {
    return false;
  }

  if (_currentTurn == Player::BLACK) {
    _blackStones.set(index);
  } else {
    _whiteStones.set(index);
  }

  // Change turn
  _currentTurn = (_currentTurn == Player::BLACK) ? Player::WHITE : Player::BLACK;
  return true;
}

Player Board::getStoneAt(int x, int y) const {
  int index = _getIndex(x, y);
  if (_blackStones.test(index))
    return Player::BLACK;
  if (_whiteStones.test(index))
    return Player::WHITE;
  return Player::NONE;
}

Player Board::getCurrentTurn() const {
  return _currentTurn;
}

bool Board::_hasFiveInARow(const std::bitset<MAX_CELLS>& stones, int shift_amount) const {
  std::bitset<MAX_CELLS> temp = stones;

  // 1回ずらしてANDをとる = 「2個並んでいる場所」が1になる
  temp &= (temp >> shift_amount);
  temp &= (temp >> shift_amount);  // 3連
  temp &= (temp >> shift_amount);  // 4連
  temp &= (temp >> shift_amount);  // 5連

  return temp.any();
}

bool Board::checkWin() {
  const auto& stones = (_currentTurn == Player::WHITE) ? _blackStones : _whiteStones;

  if (_hasFiveInARow(stones, SHIFT_H))
    return true;  // 横
  if (_hasFiveInARow(stones, SHIFT_V))
    return true;  // 縦
  if (_hasFiveInARow(stones, SHIFT_D1))
    return true;  // 右下
  if (_hasFiveInARow(stones, SHIFT_D2))
    return true;  // 左下

  return false;
}

int Board::_getIndex(int x, int y) const {
  return y * BOARD_WIDTH + x;
}
