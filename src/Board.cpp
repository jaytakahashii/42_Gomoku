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
    // ペアのパターンは2通り: (自分, 相方) or (相方, 自分)
    // 自分が index の位置にいるとして、相方が dir 方向にいるか、-dir 方向にいるか

    // --- ケース1: [index] [相方(index+dir)] ---
    int p_partner = index + dir;
    if (p_partner < MAX_CELLS && myStones.test(p_partner)) {
      // ペア発見。このペアは挟まれているか？
      // 捕獲成立条件: (敵, ペア, 空) または (空, ペア, 敵)
      // つまり、両端の一方が敵で、もう一方が空なら、相手は空に打って取れる。

      int p_end1 = index - dir;      // ペアの左側
      int p_end2 = p_partner + dir;  // ペアの右側

      // 範囲チェック
      bool end1_valid = (p_end1 >= 0 && p_end1 < MAX_CELLS);
      bool end2_valid = (p_end2 >= 0 && p_end2 < MAX_CELLS);

      // パターンA: [敵] [自] [自] [空]
      if (end1_valid && end2_valid && oppStones.test(p_end1) && !myStones.test(p_end2) &&
          !oppStones.test(p_end2)) {
        return true;
      }
      // パターンB: [空] [自] [自] [敵]
      if (end1_valid && end2_valid && !myStones.test(p_end1) && !oppStones.test(p_end1) &&
          oppStones.test(p_end2)) {
        return true;
      }
    }

    // --- ケース2: [相方(index-dir)] [index] ---
    // これは「ケース1」で dir を反転させてチェックするのと同じ
    int p_prev = index - dir;
    if (p_prev >= 0 && myStones.test(p_prev)) {
      int p_end_left = p_prev - dir;
      int p_end_right = index + dir;

      bool left_valid = (p_end_left >= 0 && p_end_left < MAX_CELLS);
      bool right_valid = (p_end_right >= 0 && p_end_right < MAX_CELLS);

      // [敵] [自] [自] [空]
      if (left_valid && right_valid && oppStones.test(p_end_left) && !myStones.test(p_end_right) &&
          !oppStones.test(p_end_right)) {
        return true;
      }
      // [空] [自] [自] [敵]
      if (left_valid && right_valid && !myStones.test(p_end_left) && !oppStones.test(p_end_left) &&
          oppStones.test(p_end_right)) {
        return true;
      }
    }
  }
  return false;
}

bool Board::checkWin() {
  const auto& myStones = (_currentTurn == Player::WHITE) ? _whiteStones : _blackStones;
  const auto& oppStones = (_currentTurn == Player::WHITE) ? _blackStones : _whiteStones;
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

int Board::_getIndex(int x, int y) const {
  return y * BOARD_WIDTH + x;
}
