#pragma once

#include <cstdint>
#include <vector>

#include "GameConfig.hpp"

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
  TranspositionTable(size_t sizeExp = 20);

  // テーブルの初期化
  void clear();

  // 値の取得
  TTEntry* get(uint64_t key);

  // 値の保存
  void store(uint64_t key, int depth, int score, TTFlag flag, Move bestMove);

 private:
  std::vector<TTEntry> _table;
  size_t _sizeMask;
};
