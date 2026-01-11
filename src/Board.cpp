#include "Board.hpp"

#include <iostream>  // TODO: デバッグ用

// ----------------------------------------------------------------
// Lifecycle & Setup
// ----------------------------------------------------------------

Board::Board()
    : _blackStones(0),
      _whiteStones(0),
      _currentTurn(Color::BLACK),
      _blackCaptures(0),
      _whiteCaptures(0),
      _capturedStatus(false),
      _doubleThreeStatus(false) {
  this->_colorToPlayer.clear();
  this->_history.clear();
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

// ----------------------------------------------------------------
// Core Gameplay Logic (Mutators)
// ----------------------------------------------------------------

bool Board::makeMove(int x, int y) {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    return false;

  int index = _getIndex(x, y);

  if (this->_blackStones.test(index) || this->_whiteStones.test(index))
    return false;

  saveState();

  _processCapture(index);
  if (!this->_capturedStatus) {
    _DoubleThree(x, y);
    if (this->_doubleThreeStatus) {
      undo();
      return false;
    }
  }

  if (this->_currentTurn == Color::BLACK) {
    this->_blackStones.set(index);
  } else {
    this->_whiteStones.set(index);
  }

  return true;
}

void Board::changeTurn() {
  this->_currentTurn = (this->_currentTurn == Color::BLACK) ? Color::WHITE : Color::BLACK;
}

bool Board::undo() {
  if (this->_history.empty())
    return false;
  BoardState s = this->_history.back();
  this->_history.pop_back();
  _applyState(s);
  return true;
}

void Board::saveState() {
  this->_history.push_back({this->_blackStones, this->_whiteStones, this->_blackCaptures,
                            this->_whiteCaptures, this->_currentTurn});
}

// ----------------------------------------------------------------
// Game Status & Win Conditions
// ----------------------------------------------------------------

bool Board::checkWin() const {
  return checkWin(_currentTurn);
}

bool Board::checkWin(Color color) const {
  const BoardType& myStones = (color == Color::WHITE) ? _whiteStones : _blackStones;
  const BoardType& oppStones = (color == Color::WHITE) ? _blackStones : _whiteStones;
  int captures = (color == Color::WHITE) ? _whiteCaptures : _blackCaptures;

  // 1. 捕獲勝ち
  if (captures >= 10)
    return true;

  // 2. 5連チェック (4方向)
  for (int shift : ALL_SHIFTS) {
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

// ----------------------------------------------------------------
// State Queries (Getters / Setters)
// ----------------------------------------------------------------

// -- Board Information --

Color Board::getColorAt(int x, int y) const {
  int index = _getIndex(x, y);
  if (this->_blackStones.test(index))
    return Color::BLACK;
  if (this->_whiteStones.test(index))
    return Color::WHITE;
  return Color::NONE;
}

Player Board::getPlayerAt(int x, int y) const {
  int index = _getIndex(x, y);
  std::map<Color, Player>::const_iterator it = this->_colorToPlayer.end();
  if (this->_blackStones.test(index))
    it = this->_colorToPlayer.find(Color::BLACK);
  if (this->_whiteStones.test(index))
    it = this->_colorToPlayer.find(Color::WHITE);
  if (it != this->_colorToPlayer.end()) {
    return it->second;
  }
  return Player::NONE;
}

// -- Stone Bitsets --

const BoardType& Board::getBlackStones() const {
  return this->_blackStones;
}

const BoardType& Board::getWhiteStones() const {
  return this->_whiteStones;
}

const BoardType& Board::getMyStones(Color myColor) const {
  return (myColor == Color::BLACK) ? _blackStones : _whiteStones;
}

const BoardType& Board::getOppStones(Color myColor) const {
  return (myColor == Color::BLACK) ? _whiteStones : _blackStones;
}

// -- Computed Bitsets --

// 有効な盤面範囲（壁以外）を表すマスクを定義
// static const にして一度だけ計算させる
static const BoardType VALID_MASK = []() {
  BoardType mask;
  for (int y = 0; y < BOARD_SIZE; ++y) {
    for (int x = 0; x < BOARD_SIZE; ++x) {
      mask.set(y * BOARD_WIDTH + x);
    }
  }
  return mask;
}();

BoardType Board::getEmptyStones() const {
  return ~(_blackStones | _whiteStones) & VALID_MASK;
}

BoardType Board::getOccupiedStones() const {
  return _blackStones | _whiteStones;
}

// -- Game State --

Color Board::getCurrentTurn() const {
  return this->_currentTurn;
}

Player Board::getCurrentPlayer() const {
  auto it = this->_colorToPlayer.find(this->_currentTurn);
  if (it == this->_colorToPlayer.end()) {
    return Player::NONE;
  }
  return it->second;
}

int Board::getBlackCaptures() const {
  return this->_blackCaptures;
}

int Board::getWhiteCaptures() const {
  return this->_whiteCaptures;
}

// -- Special Rule Flags --

bool Board::getDoubleThreeStatus() const {
  return this->_doubleThreeStatus;
}

void Board::setDoubleThreeStatus(bool status) {
  this->_doubleThreeStatus = status;
}

bool Board::getCapturedStatus() const {
  return this->_capturedStatus;
}

// -- AI Helpers --

BoardType Board::getCapturableStones(Color myColor) const {
  const BoardType& myStones = getMyStones(myColor);
  const BoardType& oppStones = getOppStones(myColor);
  BoardType capturable;

  for (int i = 0; i < MAX_CELLS; ++i) {
    if (oppStones.test(i)) {
      if (_isStoneCapturable(i, myStones, oppStones)) {
        capturable.set(i);
      }
    }
  }

  return capturable;
}

// ----------------------------------------------------------------
// Internal Helper Methods
// ----------------------------------------------------------------

// -- State Management --

void Board::_applyState(const BoardState& state) {
  this->_blackStones = state.blackStones;
  this->_whiteStones = state.whiteStones;
  this->_blackCaptures = state.blackCaptures;
  this->_whiteCaptures = state.whiteCaptures;
  this->_currentTurn = state.currentTurn;
}

// -- Coordinate / Bit Utils --

int Board::_getIndex(int x, int y) const {
  return y * BOARD_WIDTH + x;
}

// -- Rule Implementations --

void Board::_processCapture(int index) {
  BoardType& myStones = (this->_currentTurn == Color::BLACK) ? _blackStones : _whiteStones;
  BoardType& oppStones = (this->_currentTurn == Color::BLACK) ? _whiteStones : _blackStones;
  int8_t& myScore = (_currentTurn == Color::BLACK) ? _blackCaptures : _whiteCaptures;
  this->_capturedStatus = false;

  for (int d : ALL_SHIFTS) {
    const int directions[] = {d, -d};

    for (int dir : directions) {
      int p1 = index + dir;
      int p2 = index + dir * 2;
      int p3 = index + dir * 3;

      if (p3 < 0 || p3 >= MAX_CELLS)
        continue;

      // Pattern: [my(index)] [opp] [opp] [my]
      if (oppStones.test(p1) && oppStones.test(p2) && myStones.test(p3)) {
        oppStones.reset(p1);
        oppStones.reset(p2);

        myScore += 2;

        this->_capturedStatus = true;
      }
    }
  }
}

void Board::_DoubleThree(int x, int y) {
  _doubleThreeStatus = false;

  const BoardType& myStones = getMyStones(_currentTurn);
  const BoardType& oppStones = getOppStones(_currentTurn);
  int freeThreeCount = 0;

  for (const auto& dir : CHECK_DIRS) {
    // 1. Extract Line Bits
    LineBits line = _getLineBits(x, y, dir, myStones, oppStones);

    // 2. Check for Free Three pattern
    if (_checkFreeThree(line)) {
      freeThreeCount++;
      if (freeThreeCount >= 2) {
        _doubleThreeStatus = true;
      }
    }
  }
}

LineBits Board::_getLineBits(int x, int y, const Direction dir, const BoardType& myStones,
                             const BoardType& oppStones) const {
  LineBits line = {0, 0};

  // 中心 (x, y) は必ず自分の石 (bit 5)
  line.my |= (1 << 5);

  // ±5マスを走査 (i=0 は処理済みなのでスキップ可能だが、分岐減らすためループに含めても良い)
  for (int i = -5; i <= 5; ++i) {
    if (i == 0)
      continue;

    int nx = x + i * dir.dx;
    int ny = y + i * dir.dy;

    // 盤外は「敵の石（壁）」として扱う
    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) {
      line.opp |= (1 << (i + 5));
    } else {
      int idx = _getIndex(dir.dx, dir.dy);  // インライン化しても良いが、可読性優先
      if (myStones.test(idx)) {
        line.my |= (1 << (i + 5));
      } else if (oppStones.test(idx)) {
        line.opp |= (1 << (i + 5));
      }
    }
  }
  return line;
}

// 役割: ビット列が Free-Three のパターンに合致するか判定する
// 座標計算やボードデータへの依存がなくなり、純粋な論理関数になります
bool Board::_checkFreeThree(LineBits line) const {
  // パターン定義: 1=石, 0=空 (bit 0 が左端)
  // .XXX. (連三), .X.XX. (飛び三A), .XX.X. (飛び三B)
  static constexpr uint16_t patterns[] = {0b001110, 0b010110, 0b011010};

  for (const uint16_t p : patterns) {
    // パターン (6bit) をウィンドウ (11bit) 内でスライド
    for (int i = 0; i <= 5; ++i) {
      uint16_t target = p << i;
      uint16_t mask = 0b111111 << i;

      // 1. 今打った石 (bit 5) がこのパターンを構成する一部であるか？
      // これがないと「遠くにある既存の三」を誤検知してしまう
      if (!(target & (1 << 5)))
        continue;

      // 2. 自分の石の配置が一致するか？
      // (line.my & mask) == target
      // -> パターンの '1' の場所に石があり、'0' の場所には自分の石がないこと
      if ((line.my & mask) != target)
        continue;

      // 3. 敵の石（または壁）による妨害がないか？
      // マスク範囲内において、敵のビットが立っていてはならない
      if ((line.opp & mask) != 0)
        continue;

      return true;
    }
  }
  return false;
}

bool Board::_isStoneCapturable(int index, const BoardType& myStones,
                               const BoardType& oppStones) const {
  // 全方向(4軸)をチェック
  for (int dir : ALL_SHIFTS) {
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
BoardType Board::_getFiveInARowBits(const BoardType& stones, int shift_amount) const {
  BoardType temp = stones;

  // 1回ずらしてAND = 2連
  temp &= (temp >> shift_amount);
  temp &= (temp >> shift_amount);  // 3連
  temp &= (temp >> shift_amount);  // 4連
  temp &= (temp >> shift_amount);  // 5連

  return temp;
}
