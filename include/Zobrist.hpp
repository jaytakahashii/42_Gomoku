#pragma once

#include <cstdint>
#include <random>

#include "Enums.hpp"
#include "GameConfig.hpp"

class Zobrist {
 public:
  // 乱数テーブルの初期化
  static void initialize();

  // 指定したインデックスと色のハッシュ値を取得
  static uint64_t getPieceHash(int index, Color color);

  // 黒番の手番ハッシュ値を取得
  static uint64_t getBlackTurnHash();

 private:
  // 静的メンバ変数の宣言（実体はcppファイルへ）
  static uint64_t table[MAX_CELLS][2];
  static uint64_t blackTurn;
};
