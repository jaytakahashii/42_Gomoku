#pragma once

#include <cstdint>
#include <random>

#include "Enums.hpp"
#include "GameConfig.hpp"

class Zobrist {
 public:
  /**
   * @brief Initialize the Zobrist Hashing Table
   */
  static void initialize();

  /**
   * @brief Get the hash value of the piece at the specified position and color
   * @param index The index of the position
   * @param color The color of the piece
   * @return The hash value of the piece
   */
  static uint64_t getPieceHash(int index, Color color);

  /**
   * @brief Get the hash value representing the turn of black player
   */
  static uint64_t getBlackTurnHash();

 private:
  static uint64_t table[MAX_CELLS][2];
  static uint64_t blackTurn;
};
