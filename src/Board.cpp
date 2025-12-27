#include <Board.hpp>
#include <iostream>  // TODO: デバッグ用

Board::Board() : _currentTurn(Player::BLACK) {
  _blackStones.reset();
  _whiteStones.reset();
}

const std::vector<int> DIRECTIONS = {1, -1, 20, -20, 21, -21, 19, -19};

void Board::_checkAndProcessCapture(int index) {
  // 今の手番（石を置いた人）と相手のビットボードを取得
  auto& myStones = (_currentTurn == Player::BLACK) ? _blackStones : _whiteStones;
  auto& oppStones = (_currentTurn == Player::BLACK) ? _whiteStones : _blackStones;
  int& myScore = (_currentTurn == Player::BLACK) ? _blackCaptures : _whiteCaptures;

  for (int dir : DIRECTIONS) {
    // パターン: [自(index)] [敵] [敵] [自]
    int p1 = index + dir;      // 隣
    int p2 = index + dir * 2;  // 2つ隣
    int p3 = index + dir * 3;  // 3つ隣

    // ビットボードの範囲内かどうか簡易チェック（配列外アクセス防止）
    if (p3 < 0 || p3 >= MAX_CELLS)
      continue;

    // 判定ロジック:
    // 1. 隣と2つ隣が「敵」である
    // 2. 3つ隣が「自分」である
    if (oppStones.test(p1) && oppStones.test(p2) && myStones.test(p3)) {
      // 捕獲成立！敵の石を消す
      oppStones.reset(p1);
      oppStones.reset(p2);

      // スコア加算（石2個分）
      myScore += 2;

      // TODO: デバッグ用にログを出すと分かりやすい
      std::cout << "Capture! Player " << ((_currentTurn == Player::BLACK) ? "BLACK" : "WHITE")
                << std::endl;
      std::cout << "Total captures - BLACK: " << _blackCaptures << ", WHITE: " << _whiteCaptures
                << std::endl;
    }
  }
}

void Board::changeTurn() {
  _currentTurn = (_currentTurn == Player::BLACK) ? Player::WHITE : Player::BLACK;
}

int Board::getBlackCaptures() const {
  return _blackCaptures;
}

int Board::getWhiteCaptures() const {
  return _whiteCaptures;
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

  _checkAndProcessCapture(index);

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

bool Board::_hasFiveInARow(const BoardType& stones, int shift_amount) const {
  BoardType temp = stones;

  // 1回ずらしてANDをとる = 「2個並んでいる場所」が1になる
  temp &= (temp >> shift_amount);
  temp &= (temp >> shift_amount);  // 3連
  temp &= (temp >> shift_amount);  // 4連
  temp &= (temp >> shift_amount);  // 5連

  return temp.any();
}

bool Board::checkWin() {
  const BoardType& stones = (_currentTurn == Player::WHITE) ? _whiteStones : _blackStones;
  int captures = (_currentTurn == Player::WHITE) ? _whiteCaptures : _blackCaptures;

  if (captures >= 10)
    return true;

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
