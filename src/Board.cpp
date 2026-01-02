#include "Board.hpp"

#include <iostream>  // TODO: デバッグ用

Board::Board()
    : _blackStones(0),
      _whiteStones(0),
      _currentTurn(Color::BLACK),
      _blackCaptures(0),
      _whiteCaptures(0),
      _doubleThreeStatus(false) {
}

bool Board::makeMove(int x, int y) {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    return false;

  int index = _getIndex(x, y);

  if (_blackStones.test(index) || _whiteStones.test(index))
    return false;

  saveState();
  this->_capturedStatus = _checkAndProcessCapture(index);
  if (!_capturedStatus) {
    if (_isDoubleThree(x, y))
      return false;
  }

  if (_currentTurn == Color::BLACK) {
    _blackStones.set(index);
  } else {
    this->_whiteStones.set(index);
  }

  return true;
}

void Board::changeTurn() {
  _currentTurn = (_currentTurn == Color::BLACK) ? Color::WHITE : Color::BLACK;
}

bool Board::checkWin() {
  const BoardType& myStones = (_currentTurn == Color::WHITE) ? _whiteStones : _blackStones;
  const BoardType& oppStones = (_currentTurn == Color::WHITE) ? _blackStones : _whiteStones;
  int captures = (_currentTurn == Color::WHITE) ? _whiteCaptures : _blackCaptures;

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

// --- Getters / Setters ---

Color Board::getColorAt(int x, int y) const {
  int index = _getIndex(x, y);
  if (_blackStones.test(index))
    return Color::BLACK;
  if (_whiteStones.test(index))
    return Color::WHITE;
  return Color::NONE;
}

Player Board::getPlayerAt(int x, int y) const {
  int index = _getIndex(x, y);
  std::map<Color, Player>::const_iterator it = this->_colorToPlayer.end();
  if (_blackStones.test(index))
    it = this->_colorToPlayer.find(Color::BLACK);
  if (_whiteStones.test(index))
    it = this->_colorToPlayer.find(Color::WHITE);
  if (it != this->_colorToPlayer.end()) {
    return it->second;
  }
  return Player::NONE;
}

Color Board::getCurrentTurn() const {
  return _currentTurn;
}

Player Board::getCurrentPlayer() const {
  auto it = this->_colorToPlayer.find(_currentTurn);
  if (it == this->_colorToPlayer.end()) {
    return Player::NONE;
  }
  return it->second;
}

int Board::getBlackCaptures() const {
  return _blackCaptures;
}

int Board::getWhiteCaptures() const {
  return _whiteCaptures;
}

bool Board::getDoubleThreeStatus() const {
  return _doubleThreeStatus;
}

bool Board::getCapturedStatus() const {
  return this->_capturedStatus;
}

void Board::setDoubleThreeStatus(bool status) {
  _doubleThreeStatus = status;
}

void Board::setupPlayers(TurnOrder order) {
  this->_colorToPlayer.clear();
  if (order == TurnOrder::AIFirst) {
    this->_colorToPlayer.insert(std::make_pair(Color::BLACK, Player::AI));
    this->_colorToPlayer.insert(std::make_pair(Color::WHITE, Player::HUMAN));
  } else if (order == TurnOrder::HumanFirst) {
    this->_colorToPlayer.insert(std::make_pair(Color::BLACK, Player::HUMAN));
    this->_colorToPlayer.insert(std::make_pair(Color::WHITE, Player::AI));
  }
}

const BoardType& Board::getBlackStones() const {
  return _blackStones;
}

const BoardType& Board::getWhiteStones() const {
  return _whiteStones;
}

// --- Private Helpers ---

int Board::_getIndex(int x, int y) const {
  return y * BOARD_WIDTH + x;
}

bool Board::_checkAndProcessCapture(int index) {
  BoardType& myStones = (_currentTurn == Color::BLACK) ? _blackStones : _whiteStones;
  BoardType& oppStones = (_currentTurn == Color::BLACK) ? _whiteStones : _blackStones;
  int& myScore = (_currentTurn == Color::BLACK) ? _blackCaptures : _whiteCaptures;
  bool captured = false;

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

        captured = true;
      }
    }
  }
  return captured;
}

BoardType Board::_getFiveInARowBits(const BoardType& stones, int shift_amount) const {
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

bool Board::_isDoubleThree(int x, int y) {
  const BoardType& myStones = (_currentTurn == Color::BLACK) ? _blackStones : _whiteStones;
  const BoardType& oppStones = (_currentTurn == Color::BLACK) ? _whiteStones : _blackStones;

  int freeThreeCount = 0;

  // 4方向チェック
  for (auto& dir : CHECK_DIRS) {
    if (_checkFreeThree(x, y, dir.dx, dir.dy, myStones, oppStones))
      freeThreeCount++;
    if (freeThreeCount >= 2) {
      _doubleThreeStatus = true;
      break;
    }
  }

  return (freeThreeCount >= 2);
}

bool Board::_checkFreeThree(int x, int y, int dx, int dy, const BoardType& myStones,
                            const BoardType& oppStones) const {
  // bit 5 を中心 (x,y) とする
  uint16_t line_m = 0;  // m = my
  uint16_t line_o = 0;  // o = opponent

  // ±5マスを取得 (計11マス)
  // Free-Threeパターンの最大長は .X.XX. (6マス) なのでこれで十分
  for (int i = -5; i <= 5; ++i) {
    if (i == 0) {
      line_m |= (1 << 5);  // 中心は自分
      continue;
    }

    int nx = x + i * dx;
    int ny = y + i * dy;

    // 範囲外チェック
    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) {
      line_o |= (1 << (i + 5));  // 壁
    } else {
      int idx = _getIndex(nx, ny);
      if (myStones.test(idx))
        line_m |= (1 << (i + 5));
      else if (oppStones.test(idx))
        line_o |= (1 << (i + 5));
    }
  }

  // パターン定義 (1:石, 0:空)
  // bit 0 が左端
  static const uint16_t patterns[] = {
      0b001110,  // .XXX.  (連三)
      0b010110,  // .X.XX. (飛び三 A)
      0b011010   // .XX.X. (飛び三 B)
  };

  // 各パターンを盤面上でスライドさせて照合
  for (uint16_t p : patterns) {
    // パターンの長さは6ビット (0b000000 ~ 0b111111)
    // これを line_m, line_o に対してずらしながらチェック

    // パターンの位置をスライド (ウィンドウ)
    // line_m の幅は11ビット (0..10)。パターンは6ビット。
    // i はパターンの開始位置 (0..5)
    for (int i = 0; i <= 5; ++i) {
      uint16_t mask = 0b111111 << i;
      uint16_t target = p << i;

      // 1. 自分の石の形が一致するか？
      // マスク範囲内の石配置がパターンと完全一致すること
      // (パターン内の0は「石がない」ことを要求)
      if ((line_m & mask) != target)
        continue;

      // 2. 敵の石（壁）がないか？
      // マスク範囲内は敵がゼロでなければならない（両端の空も含めて）
      if ((line_o & mask) != 0)
        continue;

      // 3. 中心 (bit 5) がパターンに含まれているか？
      // 今置いた石が、そのFree-Threeの一部でなければならない
      // target (シフト済みのパターン) の bit 5 が 1 であるか確認
      if ((target & (1 << 5)) == 0)
        continue;

      // すべてクリアならFree-Three
      return true;
    }
  }

  return false;
}

void Board::saveState() {
  this->_history.push_back(
      {_blackStones, _whiteStones, _blackCaptures, _whiteCaptures, _currentTurn});
}

bool Board::undo() {
  if (_history.empty())
    return false;
  BoardState s = this->_history.back();
  this->_history.pop_back();
  _applyState(s);
  return true;
}

void Board::_applyState(BoardState state) {
  this->_blackStones = state.blackStones;
  this->_whiteStones = state.whiteStones;
  this->_blackCaptures = state.blackCaptures;
  this->_whiteCaptures = state.whiteCaptures;
  this->_currentTurn = state.currentTurn;
}
