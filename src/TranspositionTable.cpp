#include "TranspositionTable.hpp"

TranspositionTable::TranspositionTable(size_t sizeExp) {
  size_t size = 1ULL << sizeExp;
  _table.resize(size);
  _sizeMask = size - 1;
  clear();
}

void TranspositionTable::clear() {
  // 空のエントリで埋める
  // key=0, depth=-1, score=0, EXACT, {-1,0}
  std::fill(_table.begin(), _table.end(), TTEntry{0, 0, -1, TTFlag::EXACT, {-1, 0}});
}

TTEntry* TranspositionTable::get(uint64_t key) {
  // ビットマスクで高速にインデックス計算 (key % size と同じだが高速)
  size_t index = key & _sizeMask;
  TTEntry& entry = _table[index];

  // キーが一致する場合のみ返す (衝突時は無視)
  if (entry.key == key) {
    return &entry;
  }
  return nullptr;
}

void TranspositionTable::store(uint64_t key, int depth, int score, TTFlag flag, Move bestMove) {
  size_t index = key & _sizeMask;
  TTEntry& entry = _table[index];

  // 上書き戦略:
  // 1. エントリが空 (key == 0)
  // 2. キーが違う (衝突発生 -> 常に新しい方を優先)
  // 3. キーは同じだが、今回の探索の方が「深い」 (より精度の高い結果)
  if (entry.key == 0 || entry.key != key || depth >= entry.depth) {
    entry.key = key;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;
  }
}
