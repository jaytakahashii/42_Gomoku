#include "TranspositionTable.hpp"

TranspositionTable::TranspositionTable(size_t sizeExp) {
  size_t size = 1ULL << sizeExp;
  _table.resize(size);
  _sizeMask = size - 1;
  clear();
}

void TranspositionTable::clear() {
  std::fill(_table.begin(), _table.end(), TTEntry{0, 0, -1, TTFlag::EXACT, {-1, 0}});
}

TTEntry* TranspositionTable::get(uint64_t key) {
  size_t index = key & _sizeMask;
  TTEntry& entry = _table[index];

  if (entry.key == key) {
    return &entry;
  }
  return nullptr;
}

void TranspositionTable::store(uint64_t key, int depth, int score, TTFlag flag, Move bestMove) {
  size_t index = key & _sizeMask;
  TTEntry& entry = _table[index];

  if (entry.key == 0 || entry.key != key || depth >= entry.depth) {
    entry.key = key;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;
  }
}
