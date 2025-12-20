#include <Board.hpp>

Board::Board() : _currentTurn(Player::BLACK) {
  _blackStones.reset();
  _whiteStones.reset();
}

bool Board::makeMove(int x, int y) {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    return false;

  int index = getIndex(x, y);

  // Check if the cell is already occupied
  if (_blackStones.test(index) || _whiteStones.test(index)) {
    return false;
  }

  // Place the stone
  if (_currentTurn == Player::BLACK) {
    _blackStones.set(index);
    // TODO: Implement checkWin(x, y) and forbidden move checks
  } else {
    _whiteStones.set(index);
  }

  // Change turn
  _currentTurn = (_currentTurn == Player::BLACK) ? Player::WHITE : Player::BLACK;
  return true;
}

Player Board::getStoneAt(int x, int y) const {
  int index = getIndex(x, y);
  if (_blackStones.test(index))
    return Player::BLACK;
  if (_whiteStones.test(index))
    return Player::WHITE;
  return Player::NONE;
}

Player Board::getCurrentTurn() const {
  return _currentTurn;
}

bool Board::checkWin(int x, int y) {
  // TODO: Implement fast win check using bit shifting
  return false;
}

int Board::getIndex(int x, int y) const {
  return y * BOARD_WIDTH + x;
}
