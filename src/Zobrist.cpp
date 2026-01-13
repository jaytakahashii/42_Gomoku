#include "Zobrist.hpp"

// 静的メンバ変数の実体定義
// ここでメモリが確保されます（初期値は0）
uint64_t Zobrist::table[MAX_CELLS][2];
uint64_t Zobrist::blackTurn;

void Zobrist::initialize() {
  // 固定シードを使用（再現性のため）
  std::mt19937_64 rng(12345);
  std::uniform_int_distribution<uint64_t> dist;

  for (int i = 0; i < MAX_CELLS; ++i) {
    table[i][0] = dist(rng);  // 黒用の乱数
    table[i][1] = dist(rng);  // 白用の乱数
  }
  blackTurn = dist(rng);
}

uint64_t Zobrist::getPieceHash(int index, Color color) {
  // color が BLACK なら 0, WHITE なら 1
  // Color enumの実装に依存しますが、安全のため条件演算子を使用
  int colorIndex = (color == Color::BLACK) ? 0 : 1;
  return table[index][colorIndex];
}

uint64_t Zobrist::getBlackTurnHash() {
  return blackTurn;
}
