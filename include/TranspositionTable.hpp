#pragma once

#include <cstdint>
#include <vector>

#include "GameConfig.hpp"

enum class TTFlag {
  EXACT,       // (Alpha < Score < Beta)
  LOWERBOUND,  // Beta Cut (Score >= Beta)
  UPPERBOUND   // Alpha Cut (Score <= Alpha)
};

struct TTEntry {
  uint64_t key;
  int score;
  int depth;
  TTFlag flag;
  Move bestMove;
};

class TranspositionTable {
 public:
  /**
   * @brief Construct a new Transposition Table object
   */
  TranspositionTable(size_t sizeExp = 20);

  /**
   * @brief Clear the Transposition Table
   */
  void clear();

  /**
   * @brief Retrieve an entry from the Transposition Table
   * @param key The key to look up
   * @return Pointer to the TTEntry if found, otherwise nullptr
   */
  TTEntry* get(uint64_t key);

  /**
   * @brief Store an entry in the Transposition Table
   * @param key The key to store
   * @param depth The search depth
   * @param score The score to store
   * @param flag The TTFlag indicating the type of score
   * @param bestMove The best move associated with this entry
   */
  void store(uint64_t key, int depth, int score, TTFlag flag, Move bestMove);

 private:
  std::vector<TTEntry> _table;
  size_t _sizeMask;
};
