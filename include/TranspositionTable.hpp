#pragma once

#include <cstdint>
#include <vector>

#include "AI.hpp"  // Move構造体を使うため

// エントリの種類（値の信頼性）
enum class TTFlag {
  EXACT,       // 正確な値 (Alpha < Score < Beta で探索完了)
  LOWERBOUND,  // Beta Cut (Score >= Beta): これ以上探索しても無駄に高い
  UPPERBOUND   // Alpha Cut (Score <= Alpha): これ以上探索しても無駄に低い
};

struct TTEntry {
  uint64_t key;   // ハッシュキー (衝突確認用)
  int score;      // 評価値
  int depth;      // この値が見つかった時の残り深さ
  TTFlag flag;    // 値の種類
  Move bestMove;  // その時点での最善手 (Move Orderingで超重要)
};

class TranspositionTable {
 public:
  // サイズは2の累乗であること (例: 2^20 = 約100万エントリ, 32MB程度)
  TranspositionTable(size_t sizeExp = 20) {
    size_t size = 1ULL << sizeExp;
    _table.resize(size);
    _sizeMask = size - 1;
    clear();
  }

  // テーブルの初期化
  void clear() {
    // 空のエントリで埋める
    // key=0, depth=-1, score=0, EXACT, {-1,-1}
    std::fill(_table.begin(), _table.end(), TTEntry{0, 0, -1, TTFlag::EXACT, {-1, -1, 0}});
  }

  // 値の取得
  TTEntry* get(uint64_t key) {
    // ビットマスクで高速にインデックス計算 (key % size と同じだが高速)
    size_t index = key & _sizeMask;
    TTEntry& entry = _table[index];

    // キーが一致する場合のみ返す (衝突時は無視)
    if (entry.key == key) {
      return &entry;
    }
    return nullptr;
  }

  // 値の保存
  void store(uint64_t key, int depth, int score, TTFlag flag, Move bestMove) {
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

 private:
  std::vector<TTEntry> _table;
  size_t _sizeMask;
};
