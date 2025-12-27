#include "Board.hpp"

#include <iostream>  // TODO: デバッグ用

Board::Board()
    : _blackStones(0),
      _whiteStones(0),
      _currentTurn(Player::BLACK),
      _blackCaptures(0),
      _whiteCaptures(0) {
}

bool Board::makeMove(int x, int y) {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    return false;

  int index = _getIndex(x, y);

  if (_blackStones.test(index) || _whiteStones.test(index))
    return false;

  if (_currentTurn == Player::BLACK) {
    _blackStones.set(index);
  } else {
    _whiteStones.set(index);
  }

  _checkAndProcessCapture(index);

  return true;
}

void Board::changeTurn() {
  _currentTurn = (_currentTurn == Player::BLACK) ? Player::WHITE : Player::BLACK;
}

bool Board::checkWin() {
  const BoardType& myStones = (_currentTurn == Player::WHITE) ? _whiteStones : _blackStones;
  const BoardType& oppStones = (_currentTurn == Player::WHITE) ? _blackStones : _whiteStones;
  int captures = (_currentTurn == Player::WHITE) ? _whiteCaptures : _blackCaptures;

  // 1. 捕獲勝ち
  if (captures >= 10)
    return true;

  // 2. 5連チェック (4方向)
  for (int shift : ALL_DIRS) {
    // 5連の始点ビット列を取得
    BoardType lines = _getFiveInARowBits(myStones, shift);

    if (lines.none())
      continue;

    // 見つかった全ての5連ラインについて検証
    for (int i = 0; i < MAX_CELLS; ++i) {
      if (lines.test(i)) {
        // インデックス i から始まる5連が見つかった
        bool lineIsSafe = true;

        // 5つの石すべてについて「捕獲される危険性」をチェック
        for (int k = 0; k < 5; ++k) {
          int stoneIdx = i + k * shift;
          if (_isStoneCapturable(stoneIdx, myStones, oppStones)) {
            // 一つでも捕獲される石があれば、このラインでの勝利は成立しない
            lineIsSafe = false;
            std::cout << "Capturable!!" << std::endl;  // TODO: デバッグ用
            break;
          }
        }

        // 一つでも「安全な5連」があれば勝利確定
        if (lineIsSafe) {
          return true;
        }
      }
    }
  }

  return false;
}

// --- Getters ---

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

int Board::getBlackCaptures() const {
  return _blackCaptures;
}

int Board::getWhiteCaptures() const {
  return _whiteCaptures;
}

// --- Private Helpers ---

int Board::_getIndex(int x, int y) const {
  return y * BOARD_WIDTH + x;
}

void Board::_checkAndProcessCapture(int index) {
  BoardType& myStones = (_currentTurn == Player::BLACK) ? _blackStones : _whiteStones;
  BoardType& oppStones = (_currentTurn == Player::BLACK) ? _whiteStones : _blackStones;
  int& myScore = (_currentTurn == Player::BLACK) ? _blackCaptures : _whiteCaptures;

  for (int d : ALL_DIRS) {
    const int directions[] = {d, -d};

    for (int dir : directions) {
      // パターン: [自(index)] [敵] [敵] [自]
      int p1 = index + dir;      // 隣
      int p2 = index + dir * 2;  // 2つ隣
      int p3 = index + dir * 3;  // 3つ隣

      // 範囲チェック
      if (p3 < 0 || p3 >= MAX_CELLS)
        continue;

      // 判定ロジック:
      // 1. 隣と2つ隣が「敵」
      // 2. 3つ隣が「自分」
      if (oppStones.test(p1) && oppStones.test(p2) && myStones.test(p3)) {
        // 捕獲成立！敵の石を消す
        oppStones.reset(p1);
        oppStones.reset(p2);

        // スコア加算
        myScore += 2;

        // TODO: Debug output
        std::cout << "Capture! Player " << ((_currentTurn == Player::BLACK) ? "BLACK" : "WHITE")
                  << "\n"
                  << "Total captures - BLACK: " << _blackCaptures << ", WHITE: " << _whiteCaptures
                  << std::endl;
      }
    }
  }
}

Board::BoardType Board::_getFiveInARowBits(const BoardType& stones, int shift_amount) const {
  BoardType temp = stones;

  // 1回ずらしてAND = 2連
  temp &= (temp >> shift_amount);
  temp &= (temp >> shift_amount);  // 3連
  temp &= (temp >> shift_amount);  // 4連
  temp &= (temp >> shift_amount);  // 5連

  return temp;
}

bool Board::_isStoneCapturable(int index, const BoardType& myStones,
                               const BoardType& oppStones) const {
  // 全方向(4軸)をチェック
  for (int dir : ALL_DIRS) {
    // インデックスを中心とした両側 (+dir, -dir) をチェック
    const int sides[] = {dir, -dir};

    for (int d : sides) {
      int p_partner = index + d;

      // 1. 配列範囲チェック
      if (p_partner < 0 || p_partner >= MAX_CELLS)
        continue;

      // 2. 隣が自分の石(=ペア成立)かチェック
      if (myStones.test(p_partner)) {
        // ペア: [index] [p_partner]
        // このペアの両外側: (index - d) と (p_partner + d)
        int p_outer_self = index - d;
        int p_outer_partner = p_partner + d;

        // 範囲外チェック
        if (p_outer_self < 0 || p_outer_self >= MAX_CELLS)
          continue;
        if (p_outer_partner < 0 || p_outer_partner >= MAX_CELLS)
          continue;

        // 状態取得
        // 捕獲条件: (敵, ペア, 空) または (空, ペア, 敵)
        bool self_side_enemy = oppStones.test(p_outer_self);
        bool partner_side_enemy = oppStones.test(p_outer_partner);

        // 敵でなく、かつ自分の石でもなければ「空」
        bool self_side_empty = !self_side_enemy && !myStones.test(p_outer_self);
        bool partner_side_empty = !partner_side_enemy && !myStones.test(p_outer_partner);

        if ((self_side_enemy && partner_side_empty) || (self_side_empty && partner_side_enemy)) {
          return true;
        }
      }
    }
  }
  return false;
}
