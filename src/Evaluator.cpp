#include "Evaluator.hpp"

#include <algorithm>
#include <iostream>

int Evaluator::evaluate(const Board& board, Color aiColor) {
  // 1. Check for Guaranteed Win (Terminal State)
  // Always prioritize actual victory/defeat over heuristics.
  if (board.checkWin(aiColor)) {
    return ScoreConfig::WIN;
  }
  Color oppColor = (aiColor == Color::BLACK) ? Color::WHITE : Color::BLACK;
  if (board.checkWin(oppColor)) {
    return -ScoreConfig::WIN;
  }

  // 2. Prepare Bitboards
  // Separate stones into "Safe" and "Dead" (vulnerable to capture)
  const BoardType& myStones = board.getMyStones(aiColor);
  const BoardType& oppStones = board.getOppStones(aiColor);

  BoardType myDeadStones = board.getCapturableStones(aiColor);
  BoardType oppDeadStones = board.getCapturableStones(oppColor);
  BoardType empty = board.getEmptyStones();

  // Only evaluate patterns on stones that will survive the next turn
  BoardType mySafeStones = myStones & ~myDeadStones;
  BoardType oppSafeStones = oppStones & ~oppDeadStones;

  long long myScore = 0;
  long long oppScore = 0;

  // 3. Pattern Evaluation (Static)
  myScore += _evaluateColor(mySafeStones, empty);
  oppScore += _evaluateColor(oppSafeStones, empty);

  // 4. Capture Status Evaluation
  // Captures are high value, especially when close to 10.
  int myCaptures = (aiColor == Color::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (aiColor == Color::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  myScore += myCaptures * ScoreConfig::CAPTURE_SCORE;
  oppScore += oppCaptures * ScoreConfig::CAPTURE_SCORE;

  // Capture Reach (Winning soon)
  if (myCaptures >= 8)
    myScore += ScoreConfig::OPEN_FOUR;  // Huge pressure
  if (oppCaptures >= 8)
    oppScore += ScoreConfig::OPEN_FOUR;

  // 5. Threat Evaluation (Pending Captures)
  // Being under threat is severe.
  // We assume each "dead" bit is a stone. count()/2 approximates pairs.
  if (myDeadStones.any()) {
    int pairsLost = (int)myDeadStones.count();
    // Penalty: Lost stones value + giving points to opponent + positional loss
    // Multiplied to ensure AI prioritizes defense.
    myScore -= pairsLost * ScoreConfig::CAPTURE_SCORE;
  }

  if (oppDeadStones.any()) {
    int pairsTaken = (int)oppDeadStones.count();
    // Bonus: Gaining stones + positional advantage
    myScore += pairsTaken * ScoreConfig::CAPTURE_SCORE;
  }

  // 6. Final Calculation
  // Apply defensive multiplier using integer math: x * 1.2  == x + x/5
  long long weightedOppScore = oppScore + (oppScore / 5);

  return static_cast<int>(myScore - weightedOppScore);
}

int Evaluator::evaluateMovePriority(const Board& board, int index, Color myColor) {
  int score = 0;

  // 1. Centrality Bonus (Optional but recommended)
  int x = index & WIDTH_MASK;    // index % 32
  int y = index >> WIDTH_SHIFT;  // index / 32

  int centerDist = std::abs(x - BOARD_SIZE / 2) + std::abs(y - BOARD_SIZE / 2);
  score += (10 - centerDist);

  const BoardType& myStones = board.getMyStones(myColor);
  const BoardType& oppStones = board.getOppStones(myColor);
  const BoardType& sentinels = board.getSentinelStones();

  // 2. Line Analysis with Offsets

  // Horizontal (Offset 1)
  score += _CheckLineScore(index, 1, myStones, oppStones, sentinels);
  // Vertical (Offset 32)
  score += _CheckLineScore(index, BOARD_WIDTH, myStones, oppStones, sentinels);
  // Diagonal \ (Offset 33: 1 down + 1 right)
  score += _CheckLineScore(index, BOARD_WIDTH + 1, myStones, oppStones, sentinels);
  // Diagonal / (Offset 31: 1 down + 1 left)
  score += _CheckLineScore(index, BOARD_WIDTH - 1, myStones, oppStones, sentinels);

  return score;
}

int Evaluator::_CheckLineScore(int index, int offset, const BoardType& myStones,
                               const BoardType& oppStones, const BoardType& sentinels) {
  // 0: My stones, 1: Opponent stones
  int consecutive[2] = {0, 0};
  int openEnds[2] = {0, 0};

  // direction: 1 (Forward), -1 (Backward)
  for (int sign = -1; sign <= 1; sign += 2) {
    bool myBlocked = false;
    bool oppBlocked = false;

    for (int k = 1; k <= 4; ++k) {
      int currentIdx = index + (offset * k * sign);

      if (currentIdx < 0 || currentIdx >= MAX_CELLS || sentinels.test(currentIdx))
        break;

      // --- Check My Stones ---
      bool isMyStone = myStones.test(currentIdx);
      bool isOppStone = oppStones.test(currentIdx);

      if (!myBlocked) {
        if (isMyStone) {
          consecutive[0]++;
        } else {
          if (!isOppStone)  // Empty
            openEnds[0]++;
          myBlocked = true;
        }
      }

      // --- Check Opponent Stones ---
      if (!oppBlocked) {
        if (isOppStone) {
          consecutive[1]++;
        } else {
          if (!isMyStone)  // Empty
            openEnds[1]++;
          oppBlocked = true;
        }
      }

      if (myBlocked && oppBlocked)
        break;
    }
  }

  // --- Scoring Logic  ---
  int score = 0;

  // 1. My Offense
  int myTotal = consecutive[0] + 1;
  if (myTotal >= 5)
    score += ScoreConfig::PRIORITY_WIN;  // XXXXX
  else if (myTotal == 4) {
    if (openEnds[0] >= 2)
      score += ScoreConfig::PRIORITY_OPEN_FOUR;  // .XXXX.
    else if (openEnds[0] >= 1)
      score += ScoreConfig::PRIORITY_OPEN_THREE;  // XXXX.
  } else if (myTotal == 3) {
    if (openEnds[0] >= 2)
      score += ScoreConfig::PRIORITY_OPEN_THREE;  // .XXX.
  }

  // 2. Opponent Defense
  int oppTotal = consecutive[1] + 1;
  if (oppTotal >= 5)
    score += ScoreConfig::PRIORITY_BLOCK_WIN;  // XXXXX
  else if (oppTotal == 4) {
    if (openEnds[1] >= 2)
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_4;  // .XXXX.
    else if (openEnds[1] >= 1)
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_4;  // XXXX. (Force block)
  } else if (oppTotal == 3) {
    if (openEnds[1] >= 2)
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_3;  // .XXX.
  }

  return score;
}

int Evaluator::_evaluateColor(const BoardType& stones, const BoardType& empty) {
  int score = 0;

  // Pre-calculate shifted patterns involves some redundant work if done inside the loop,
  // but the compiler will unroll and optimize this effectively.
  for (int s : ALL_DIRS) {
    // Shifted Bitboards
    // sN means stones shifted left by N steps (looking N steps ahead)
    BoardType s1 = stones >> s;
    BoardType s2 = stones >> (2 * s);
    BoardType s3 = stones >> (3 * s);
    BoardType s4 = stones >> (4 * s);
    // Note: s5 is not needed because we don't count "Breakable Fives" here.
    // If a 5 existed and was unbreakable, checkWin() would have caught it.

    // Empty shifts
    BoardType e0 = empty;  // Empty at current
    BoardType e1 = empty >> s;
    BoardType e2 = empty >> (2 * s);
    BoardType e3 = empty >> (3 * s);
    BoardType e4 = empty >> (4 * s);
    BoardType e5 = empty >> (5 * s);
    // BoardType e6 = empty >> (6 * s); // Needed for .XXXX. context?

    // --- Priority S: Open Four (.XXXX.) ---
    // Pattern: . X X X X .
    // Indices: 0 1 2 3 4 5  (relative to e0)
    BoardType openFour = e0 & s1 & s2 & s3 & s4 & e5;

    if (openFour.any()) {
      score += (int)openFour.count() * ScoreConfig::OPEN_FOUR;
    }

    // --- Priority A: Closed Four ---
    // We must avoid double counting OpenFour as ClosedFour.

    // Type 1: .XXXX (Blocked on right or edge)
    // Pattern: . X X X X (Not followed by .)
    // Match: e0 & s1 & s2 & s3 & s4
    BoardType closedFour1 = e0 & s1 & s2 & s3 & s4;

    // Remove OpenFours from this set
    // OpenFour (.XXXX.) is a subset of ClosedFour1 (.XXXX)
    if (openFour.any()) {
      closedFour1 &= ~openFour;
    }

    // Type 2: XXXX. (Blocked on left or edge)
    // Pattern: X X X X .
    // This pattern starts at the STONE, not the empty space.
    // Match: stones & s1 & s2 & s3 & e4
    BoardType closedFour2 = stones & s1 & s2 & s3 & e4;

    // Remove OpenFours from this set
    // If OpenFour exists at index i (.XXXX.), then ClosedFour2 exists at index i+s (XXXX.)
    // We need to mask out the bits in closedFour2 that correspond to shifted openFour bits.
    // Mask = openFour << s
    if (openFour.any()) {
      closedFour2 &= ~(openFour << s);
    }

    // Split Fours (Broken patterns)
    // XX.XX
    BoardType splitFour1 = stones & s1 & e2 & s3 & s4;
    // X.XXX
    BoardType splitFour2 = stones & e1 & s2 & s3 & s4;
    // XXX.X
    BoardType splitFour3 = stones & s1 & s2 & e3 & s4;

    int cfCount = (int)(closedFour1.count() + closedFour2.count() + splitFour1.count() +
                        splitFour2.count() + splitFour3.count());
    if (cfCount > 0) {
      score += cfCount * ScoreConfig::CLOSED_FOUR;
    }

    // --- Priority B: Open Three (.XXX.) ---
    // Pattern: . X X X .
    // Match: e0 & s1 & s2 & s3 & e4
    BoardType openThree = e0 & s1 & s2 & s3 & e4;

    // Note: OpenThree (.XXX.) often overlaps with OpenFour (.XXXX.)?
    // No, .XXXX. has 4 stones. .XXX. has 3. Length differs.
    // However, we should check if .XXX. is actually part of .XXXX. (which we already counted)
    // But s4 & e5 in OpenFour vs e4 in OpenThree distinguishes them.
    // If it's .XXXX., then at e4 there is a stone, so OpenThree (.XXX.) fails.
    // So they are mutually exclusive. Safe.

    // Split Open Threes
    // .X.XX.
    BoardType brokenThree1 = e0 & s1 & e2 & s3 & s4 & e5;
    // .XX.X.
    BoardType brokenThree2 = e0 & s1 & s2 & e3 & s4 & e5;

    int otCount = (int)(openThree.count() + brokenThree1.count() + brokenThree2.count());
    if (otCount > 0) {
      score += otCount * ScoreConfig::OPEN_THREE;
    }

    // Priority C: Open Two (.XX.)
    // Can be added if needed, but keeping it light for now.
  }
  return score;
}
