#include "Board.hpp"

// ----------------------------------------------------------------
// Lifecycle & Setup
// ----------------------------------------------------------------

Board::Board()
    : _handCount(0),
      _currentTurn(Color::BLACK),
      _blackCaptures(0),
      _whiteCaptures(0),
      _capturedStatus(false),
      _doubleThreeStatus(false),
      _currentHash(0),
      _prePlayerCannotMove(false) {
  this->_blackStones.reset();
  this->_whiteStones.reset();

  this->_sentinelStones.reset();
  for (int y = 0; y < BOARD_SIZE; ++y) {
    for (int x = BOARD_SIZE; x < BOARD_WIDTH; ++x) {
      int index = y * BOARD_WIDTH + x;
      _sentinelStones.set(index);
    }
  }

  this->_forbiddenHandsOfPro.reset();
  for (int y = (BOARD_SIZE / 2) - 2; y <= (BOARD_SIZE / 2) + 2; ++y) {
    for (int x = (BOARD_SIZE / 2) - 2; x <= (BOARD_SIZE / 2) + 2; ++x) {
      int index = y * BOARD_WIDTH + x;
      _forbiddenHandsOfPro.set(index);
    }
  }

  this->_forbiddenHandsOfLongPro.reset();
  for (int y = (BOARD_SIZE / 2) - 3; y <= (BOARD_SIZE / 2) + 3; ++y) {
    for (int x = (BOARD_SIZE / 2) - 3; x <= (BOARD_SIZE / 2) + 3; ++x) {
      int index = y * BOARD_WIDTH + x;
      _forbiddenHandsOfLongPro.set(index);
    }
  }

  this->_colorToPlayer.clear();
  this->_history.clear();
  this->_aiHistory.clear();
  this->_aiHistory.reserve(100);
}

void Board::setupPlayers(TurnOrder order, bool isPvP) {
  this->_colorToPlayer.clear();
  if (isPvP) {
    this->_colorToPlayer.insert(std::make_pair(Color::BLACK, Player::HUMAN));
    this->_colorToPlayer.insert(std::make_pair(Color::WHITE, Player::HUMAN));
  } else {
    if (order == TurnOrder::AIFirst) {
      this->_colorToPlayer.insert(std::make_pair(Color::BLACK, Player::AI));
      this->_colorToPlayer.insert(std::make_pair(Color::WHITE, Player::HUMAN));
    } else if (order == TurnOrder::HumanFirst) {
      this->_colorToPlayer.insert(std::make_pair(Color::BLACK, Player::HUMAN));
      this->_colorToPlayer.insert(std::make_pair(Color::WHITE, Player::AI));
    }
  }
}

// ----------------------------------------------------------------
// Core Gameplay Logic (Mutators)
// ----------------------------------------------------------------

bool Board::canMove() {
  BoardType emptyStones = getEmptyStones();
  for (int i = 0; i < MAX_CELLS; ++i) {
    if (emptyStones.test(i)) {
      _DoubleThree(i);
      if (!_doubleThreeStatus) {
        return true;
      }
    }
  }
  return false;
}

bool Board::makeMove(int index) {
  if (index < 0 || index >= MAX_CELLS || this->_sentinelStones.test(index))
    return false;

  if (this->_blackStones.test(index) || this->_whiteStones.test(index))
    return false;

  BoardType occupied = getOccupiedStones();
  this->_forbiddenHandStatus = false;

  if (_openingRule == OpeningRule::Pro) {
    if (occupied.none()) {
      // First move must be center
      if (index != CENTER_INDEX) {
        this->_forbiddenHandStatus = true;
        return false;
      }
    } else if (occupied.count() == 2) {
      // Second move must be within forbidden hands area
      if (this->_forbiddenHandsOfPro.test(index)) {
        this->_forbiddenHandStatus = true;
        return false;
      }
    }
  } else if (_openingRule == OpeningRule::LongPro) {
    if (occupied.none()) {
      // First move must be center
      if (index != CENTER_INDEX) {
        this->_forbiddenHandStatus = true;
        return false;
      }
    } else if (occupied.count() == 2) {
      // Second move must be within forbidden hands area
      if (this->_forbiddenHandsOfLongPro.test(index)) {
        this->_forbiddenHandStatus = true;
        return false;
      }
    }
  }

  saveState();

  if (this->_currentTurn == Color::BLACK) {
    this->_blackStones.set(index);
  } else {
    this->_whiteStones.set(index);
  }

  // Update hash
  this->_currentHash ^= Zobrist::getPieceHash(index, this->_currentTurn);

  _processCapture(index, nullptr);

  if (!this->_capturedStatus) {
    _DoubleThree(index);
    if (this->_doubleThreeStatus) {
      undo();
      return false;
    }
  }

  _handCount++;

  _prePlayerCannotMove = false;

  return true;
}

bool Board::makeMoveAI(int index) {
  if (index < 0 || index >= MAX_CELLS || this->_sentinelStones.test(index))
    return false;

  if (this->_blackStones.test(index) || this->_whiteStones.test(index))
    return false;

  // create record for undo
  AIMoveRecord record;
  record.moveIndex = index;
  record.prevHash = this->_currentHash;
  record.currentTurn = this->_currentTurn;
  record.prevBlackCaptures = this->_blackCaptures;
  record.prevWhiteCaptures = this->_whiteCaptures;
  record.capturedCount = 0;

  if (this->_currentTurn == Color::BLACK) {
    this->_blackStones.set(index);
  } else {
    this->_whiteStones.set(index);
  }

  // Update hash
  this->_currentHash ^= Zobrist::getPieceHash(index, this->_currentTurn);

  _processCapture(index, &record);

  this->_aiHistory.push_back(record);

  if (!this->_capturedStatus) {
    _DoubleThree(index);
    if (this->_doubleThreeStatus) {
      undoAI();
      return false;
    }
  }

  return true;
}

void Board::changeTurn() {
  _currentTurn = static_cast<Color>(3 ^ static_cast<int>(_currentTurn));
  this->_currentHash ^= Zobrist::getBlackTurnHash();
}

bool Board::undo() {
  if (this->_history.empty())
    return false;
  BoardState s = this->_history.back();
  this->_history.pop_back();
  _applyState(s);
  return true;
}

void Board::undoAI() {
  if (this->_aiHistory.empty())
    return;

  AIMoveRecord record = this->_aiHistory.back();
  this->_aiHistory.pop_back();

  this->_currentTurn = record.currentTurn;

  if (record.currentTurn == Color::BLACK) {
    this->_blackStones.reset(record.moveIndex);
  } else {
    this->_whiteStones.reset(record.moveIndex);
  }

  Color oppColor = (record.currentTurn == Color::BLACK) ? Color::WHITE : Color::BLACK;
  BoardType* oppBoard = (oppColor == Color::BLACK) ? &this->_blackStones : &this->_whiteStones;

  for (int i = 0; i < record.capturedCount; ++i) {
    int capIndex = record.capturedIndices[i];
    oppBoard->set(capIndex);
  }

  // 5. カウンターとハッシュを復元
  this->_blackCaptures = record.prevBlackCaptures;
  this->_whiteCaptures = record.prevWhiteCaptures;
  this->_currentHash = record.prevHash;
}

void Board::saveState() {
  this->_history.push_back({this->_blackStones, this->_whiteStones, this->_blackCaptures,
                            this->_whiteCaptures, this->_currentTurn, this->_currentHash,
                            this->_handCount});
}

// ----------------------------------------------------------------
// Game Status & Win Conditions
// ----------------------------------------------------------------

bool Board::checkWin() const {
  return checkWin(_currentTurn);
}

bool Board::checkWin(Color color) const {
  // 1. Win by captures
  if (_getCaptureCount(color) >= 10)
    return true;

  // 2. Check for 5-in-a-row (in 4 directions)
  const BoardType& myStones = getMyStones(color);
  const BoardType& oppStones = getOppStones(color);

  for (int shift : DIR_OFFSETS) {
    // Get the starting bitset for 5-in-a-row
    BoardType lines = _getFiveInARowBits(myStones, shift);

    if (lines.none())
      continue;

    // Validate all found 5-in-a-row lines
    for (int i = 0; i < MAX_CELLS; ++i) {
      if (lines.test(i)) {
        // Endgame Capture Rule:
        // A line of 5 wins ONLY if the opponent cannot break it by capturing a pair.
        if (_isWinningLineSafe(i, shift, myStones, oppStones)) {
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
  int index = getIndex(x, y);
  if (this->_blackStones.test(index))
    return Color::BLACK;
  if (this->_whiteStones.test(index))
    return Color::WHITE;
  return Color::NONE;
}

Player Board::getPlayerAt(int x, int y) const {
  int index = getIndex(x, y);
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

const BoardType& Board::getSentinelStones() const {
  return this->_sentinelStones;
}

// -- Computed Bitsets --

// 有効な盤面範囲（壁以外）を表すマスクを定義
// static const にして一度だけ計算させる
const BoardType Board::_validMask = []() {
  BoardType mask;
  for (int y = 0; y < BOARD_SIZE; ++y) {
    for (int x = 0; x < BOARD_SIZE; ++x) {
      mask.set(y * BOARD_WIDTH + x);
    }
  }
  return mask;
}();

BoardType Board::getEmptyStones() const {
  return ~(_blackStones | _whiteStones) & _validMask;
}

BoardType Board::getOccupiedStones() const {
  return _blackStones | _whiteStones;
}

// -- Game State --

int Board::getHandCount() const {
  return this->_handCount;
}

Color Board::getCurrentTurn() const {
  return this->_currentTurn;
}

Color Board::getNextTurn() const {
  return static_cast<Color>(3 ^ static_cast<int>(_currentTurn));
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

bool Board::isPrePlayerCannotMove() const {
  return this->_prePlayerCannotMove;
}

void Board::setPrePlayerCannotMove(bool status) {
  this->_prePlayerCannotMove = status;
}

// -- Special Rule Flags --

void Board::setOpeningRule(OpeningRule rule) {
  this->_openingRule = rule;
}

OpeningRule Board::getOpeningRule() const {
  return this->_openingRule;
}

bool Board::getDoubleThreeStatus() const {
  return this->_doubleThreeStatus;
}

void Board::setDoubleThreeStatus(bool status) {
  this->_doubleThreeStatus = status;
}

bool Board::getCapturedStatus() const {
  return this->_capturedStatus;
}

bool Board::getForbiddenHandStatus() const {
  return this->_forbiddenHandStatus;
}

void Board::setForbiddenHandStatus(bool status) {
  this->_forbiddenHandStatus = status;
}

// -- Hashing --
uint64_t Board::getHash() const {
  return this->_currentHash;
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

// -- Helpers --

int Board::getIndex(int x, int y) const {
  return y * BOARD_WIDTH + x;
}

std::pair<int, int> Board::getCoordinates(int index) const {
  int y = index / BOARD_WIDTH;
  int x = index % BOARD_WIDTH;
  return {x, y};
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
  this->_currentHash = state.hash;
  this->_handCount = state.handCount;
}

// -- Coordinate / Bit Utils --

int8_t Board::_getCaptureCount(Color color) const {
  return (color == Color::BLACK) ? this->_blackCaptures : this->_whiteCaptures;
}

// -- Rule Implementations --

void Board::_processCapture(int index, AIMoveRecord* record) {
  BoardType& myStones = (this->_currentTurn == Color::BLACK) ? _blackStones : _whiteStones;
  BoardType& oppStones = (this->_currentTurn == Color::BLACK) ? _whiteStones : _blackStones;
  int8_t& myScore = (_currentTurn == Color::BLACK) ? _blackCaptures : _whiteCaptures;
  Color oppColor = (this->_currentTurn == Color::BLACK) ? Color::WHITE : Color::BLACK;
  this->_capturedStatus = false;

  for (int d : DIR_OFFSETS) {
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
        _currentHash ^= Zobrist::getPieceHash(p1, oppColor);
        _currentHash ^= Zobrist::getPieceHash(p2, oppColor);

        myScore += 2;

        this->_capturedStatus = true;

        if (record) {
          if (record->capturedCount + 2 <= 8) {
            record->capturedIndices[record->capturedCount++] = p1;
            record->capturedIndices[record->capturedCount++] = p2;
          }
        }
      }
    }
  }
}

void Board::_DoubleThree(int index) {
  _doubleThreeStatus = false;

  const BoardType& myStones = getMyStones(_currentTurn);
  const BoardType& oppStones = getOppStones(_currentTurn);
  int freeThreeCount = 0;

  for (int offset : DIR_OFFSETS) {
    // 1. Extract Line Bits
    LineBits line = _getLineBits(index, offset, myStones, oppStones);

    // 2. Check for Free Three pattern
    if (_checkFreeThree(line)) {
      freeThreeCount++;
      if (freeThreeCount >= 2) {
        _doubleThreeStatus = true;
        return;
      }
    }
  }
}

LineBits Board::_getLineBits(int centerIndex, int offset, const BoardType& myStones,
                             const BoardType& oppStones) const {
  LineBits line = {0, 0};

  // Center bit (the placed stone)
  line.my |= (1 << 5);

  // Check in both directions
  for (int i = -5; i <= 5; ++i) {
    if (i == 0)
      continue;

    int targetIndex = centerIndex + (i * offset);

    // Treat out-of-bounds as "enemy stone (wall)"
    if (targetIndex < 0 || targetIndex >= MAX_CELLS || _sentinelStones.test(targetIndex)) {
      line.opp |= (1 << (i + 5));  // 壁は敵石扱い
      continue;
    }
    if (myStones.test(targetIndex)) {
      line.my |= (1 << (i + 5));
    } else if (oppStones.test(targetIndex)) {
      line.opp |= (1 << (i + 5));
    }
  }

  return line;
}

bool Board::_checkFreeThree(LineBits line) const {
  // Pattern definition: 1=stone, 0=empty (bit 0 is the leftmost)
  // .XXX. (three in a row), .X.XX. (jumping three A), .XX.X. (jumping three B)
  static constexpr uint16_t patterns[] = {0b001110, 0b010110, 0b011010};

  for (const uint16_t p : patterns) {
    // Slide the pattern (6bit) within the window (11bit)
    for (int i = 0; i <= 5; ++i) {
      uint16_t target = p << i;
      uint16_t mask = 0b111111 << i;

      // 1. Check if the recently placed stone (bit 5) is part of this pattern
      if (!(target & (1 << 5)))
        continue;

      // 2. Check if my stone's placement matches
      // (line.my & mask) == target
      // -> There should be stones at '1' positions and no stones at '0' positions
      if ((line.my & mask) != target)
        continue;

      // 3. Check for interference from enemy stones (or walls)
      // There should be no enemy bits within the mask range
      if ((line.opp & mask) != 0)
        continue;

      return true;
    }
  }
  return false;
}

bool Board::_isWinningLineSafe(int startIdx, int shift, const BoardType& myStones,
                               const BoardType& oppStones) const {
  // Check each of the 5 stones in the line
  for (int k = 0; k < 5; ++k) {
    int stoneIdx = startIdx + k * shift;

    // If the opponent can capture a pair that includes this stone,
    // the line is considered "breakable" and does not count as a win yet. [cite: 25]
    if (_isStoneCapturable(stoneIdx, myStones, oppStones)) {
      return false;
    }
  }
  return true;
}

bool Board::_isStoneCapturable(int index, const BoardType& myStones,
                               const BoardType& oppStones) const {
  for (int dir : DIR_OFFSETS) {
    const int neighbors[] = {dir, -dir};

    for (int d : neighbors) {
      int partner = index + d;

      // 1. Basic Validity Check:
      // Is the neighbor within bounds and is it my stone?
      if (partner < 0 || partner >= MAX_CELLS || !myStones.test(partner)) {
        continue;
      }

      // We have a pair: [index]-[partner].
      // Now check the outer flanks: (flank1) [index] [partner] (flank2)
      int flank1 = index - d;    // The side next to 'index'
      int flank2 = partner + d;  // The side next to 'partner'

      // 2. Boundary Check for flanks
      if (flank1 < 0 || flank1 >= MAX_CELLS || flank2 < 0 || flank2 >= MAX_CELLS) {
        continue;
      }

      // 3. Capture Threat Check:
      // Pattern must be: (Enemy, Pair, Empty) OR (Empty, Pair, Enemy)
      bool f1_enemy = oppStones.test(flank1);
      bool f2_enemy = oppStones.test(flank2);

      // Optimization: If both are enemies (XOOX) -> Already captured (should have been removed)
      //               If neither are enemies   -> Safe for now
      // We only care if EXACTLY one flank is an enemy.
      if (f1_enemy == f2_enemy) {
        continue;
      }

      // Now we know exactly one side is Enemy.
      // We just need to verify the *other* side is Empty (not my stone).
      // (Note: We don't need to check oppStones for the empty side because f1!=f2 guarantees it's
      // not enemy)

      if (f1_enemy) {
        // flank1 is Enemy, so flank2 MUST be Empty
        if (!myStones.test(flank2))
          return true;
      } else {
        // flank2 is Enemy, so flank1 MUST be Empty
        if (!myStones.test(flank1))
          return true;
      }
    }
  }
  return false;
}

BoardType Board::_getFiveInARowBits(const BoardType& stones, int shift_amount) const {
  BoardType temp = stones;

  temp &= (temp >> shift_amount);
  temp &= (temp >> shift_amount);
  temp &= (temp >> shift_amount);
  temp &= (temp >> shift_amount);

  return temp;
}
