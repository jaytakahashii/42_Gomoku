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
    int pairsLost = (int)myDeadStones.count() / 2;
    // Penalty: Lost stones value + giving points to opponent + positional loss
    // Multiplied to ensure AI prioritizes defense.
    myScore -= pairsLost * (ScoreConfig::CAPTURE_SCORE * 2 + ScoreConfig::OPEN_THREE);
  }

  if (oppDeadStones.any()) {
    int pairsTaken = (int)oppDeadStones.count() / 2;
    // Bonus: Gaining stones + positional advantage
    myScore += pairsTaken * (ScoreConfig::CAPTURE_SCORE + ScoreConfig::OPEN_TWO);
  }

  // 6. Final Calculation
  // Apply defensive multiplier using integer math: x * 1.2  == x + x/5
  long long weightedOppScore = oppScore + (oppScore / 5);

  return static_cast<int>(myScore - weightedOppScore);
}

int Evaluator::evaluateMovePriority(const Board& board, int x, int y, Color myColor) {
  int score = 0;
  Color oppColor = (myColor == Color::BLACK) ? Color::WHITE : Color::BLACK;

  // 1. Centrality Bonus (Optional but recommended)
  // Encourages play in the center early game.
  int centerDist = std::abs(x - BOARD_SIZE / 2) + std::abs(y - BOARD_SIZE / 2);
  score += (10 - centerDist);  // Small bonus (0-10 points)

  // 2. Capture Heuristic (Crucial for Ninuki-Renju)
  // If this move captures something, give it a HUGE bonus.
  // It's worth checking even if slightly expensive because captures alter the board state
  // significantly. (Assuming _CheckCapture is a lightweight helper you might add, or rely on line
  // checks) For now, we stick to line checks as requested.

  // 3. Line Analysis
  // [1,0], [0,1], [1,1], [1,-1]
  // Unrolling the loop manually for performance as you did is good.
  score += _CheckLineScore(board, x, y, 1, 0, myColor, oppColor);
  score += _CheckLineScore(board, x, y, 0, 1, myColor, oppColor);
  score += _CheckLineScore(board, x, y, 1, 1, myColor, oppColor);
  score += _CheckLineScore(board, x, y, 1, -1, myColor, oppColor);

  return score;
}

int Evaluator::_CheckLineScore(const Board& board, int x, int y, int dx, int dy, Color myColor,
                               Color oppColor) {
  int score = 0;

  // We need to count consecutive stones in both directions
  // 0: My stones, 1: Opponent stones
  int consecutive[2] = {0, 0};
  int openEnds[2] = {0, 0};

  // Helper lambda to scan a direction
  auto scan = [&](int k_start, int k_end, int sign) {
    bool myBlocked = false;
    bool oppBlocked = false;

    for (int k = k_start; k <= k_end; ++k) {
      int nx = x + (dx * k * sign);
      int ny = y + (dy * k * sign);

      if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) {
        // Board edge counts as blocked
        return;
      }

      Color c = board.getColorAt(nx, ny);

      // --- Check My Stones ---
      if (!myBlocked) {
        if (c == myColor) {
          consecutive[0]++;
        } else {
          if (c == Color::NONE)
            openEnds[0]++;
          myBlocked = true;  // Stop counting my consecutive
        }
      }

      // --- Check Opponent Stones ---
      // We are checking: "If I hadn't played here, how many would opp have?"
      if (!oppBlocked) {
        if (c == oppColor) {
          consecutive[1]++;
        } else {
          if (c == Color::NONE)
            openEnds[1]++;
          oppBlocked = true;
        }
      }

      if (myBlocked && oppBlocked)
        break;
    }
  };

  // Scan Forward and Backward
  scan(1, 4, 1);   // Forward (Limit 4 is enough to detect 5)
  scan(1, 4, -1);  // Backward

  // --- Scoring Logic ---

  // 1. My Offense (Trying to build lines)
  // +1 because we are placing a stone at (x,y)
  int myTotal = consecutive[0] + 1;

  if (myTotal >= 5) {
    score += ScoreConfig::PRIORITY_WIN;  // 5連
  } else if (myTotal == 4) {
    if (openEnds[0] >= 2)
      score += ScoreConfig::PRIORITY_OPEN_FOUR;  // .XXXX.
    else if (openEnds[0] >= 1)
      score += ScoreConfig::PRIORITY_OPEN_THREE;  // Closed 4 (still strong)
  } else if (myTotal == 3) {
    if (openEnds[0] >= 2)
      score += ScoreConfig::PRIORITY_OPEN_THREE;  // .XXX.
  }

  // 2. Opponent Defense (Blocking their lines)
  // +1 because if we don't play here, they will play here and connect their lines
  int oppTotal = consecutive[1] + 1;

  if (oppTotal >= 5) {
    score += ScoreConfig::PRIORITY_BLOCK_WIN;  // Block their 5
  } else if (oppTotal == 4) {
    // Blocking a 4 is critical. Even a closed 4 is deadly if not blocked.
    if (openEnds[1] >= 2)
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_4;  // Block .XXXX.
    else if (openEnds[1] >= 1)
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_4;  // Block X.XXX (Force block)
  } else if (oppTotal == 3) {
    if (openEnds[1] >= 2)
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_3;  // Block .XXX.
  }

  return score;
}

int Evaluator::_CountPatterns(const BoardType& stones, const BoardType& empty) {
  int score = 0;

  for (int s : ALL_DIRS) {
    // シフト作成
    BoardType s1 = stones >> s;
    BoardType s2 = stones >> (2 * s);
    BoardType s3 = stones >> (3 * s);
    BoardType s4 = stones >> (4 * s);
    BoardType s5 = stones >> (5 * s);

    // 空点シフト
    BoardType e0 = empty;  // 現在地
    BoardType e1 = empty >> s;
    BoardType e2 = empty >> (2 * s);
    BoardType e3 = empty >> (3 * s);
    BoardType e4 = empty >> (4 * s);
    BoardType e5 = empty >> (5 * s);

    // --- Priority S+: Five (XXXXX) ---

    BoardType five = s1 & s2 & s3 & s4 & s5;

    if (five.any()) {
      score += (int)five.count() * ScoreConfig::FIVE;
    }

    // --- Priority S: Open Four (.XXXX.) ---
    // パターン: e0 & X & X & X & X & e5
    // 基準ビットは e0 の位置
    BoardType openFour = e0 & s1 & s2 & s3 & s4 & e5;

    // --- Priority A: Closed Four (XXXX.) / (.XXXX) / (XX.XX) etc ---
    // OpenFourでカウントした場所を除外するためにマスクを作成
    // openFourのビットが立っている場所は e0 なので、そのライン上の石は ClosedFour
    // として数えないようにする
    // ただし厳密な除外は複雑なので、ここでは「OpenFourが見つかった始点」を避ける簡易実装とする

    // Pattern: X X X X . (e0を含まない、石の始点基準)
    // stones & s1(X) & s2(X) & s3(X) & e4(.)
    BoardType closedFour1 = stones & s1 & s2 & s3 & e4;

    // Pattern: . X X X X (基準 e0)
    // e0 & s1 & s2 & s3 & s4
    BoardType closedFour2 = e0 & s1 & s2 & s3 & s4;
    if (openFour.any()) {
      // openFour (.XXXX.) は closedFour2 (.XXXX) にもマッチしてしまうので、その分ビットを消す
      closedFour2 &= ~openFour;
    }

    // Split patterns (XX.XX), (X.XXX), (XXX.X)
    BoardType splitFour1 = stones & s1 & e2 & s3 & s4;  // XX.XX
    BoardType splitFour2 = stones & e1 & s2 & s3 & s4;  // X.XXX
    BoardType splitFour3 = stones & s1 & s2 & e3 & s4;  // XXX.X

    // --- Priority B: Open Three (.XXX.) ---
    // Pattern: . X X X .
    BoardType openThree = e0 & s1 & s2 & s3 & e4;

    // Split Open Three (.X.XX.) (.XX.X.)
    BoardType brokenThree1 = e0 & s1 & e2 & s3 & s4 & e5;  // .X.XX.
    BoardType brokenThree2 = e0 & s1 & s2 & e3 & s4 & e5;  // .XX.X.

    // --- 加点 ---
    if (openFour.any())
      score += (int)openFour.count() * ScoreConfig::OPEN_FOUR;

    int cfCount = (int)(closedFour1.count() + closedFour2.count() + splitFour1.count() +
                        splitFour2.count() + splitFour3.count());
    if (cfCount > 0)
      score += cfCount * ScoreConfig::CLOSED_FOUR;

    int otCount = (int)(openThree.count() + brokenThree1.count() + brokenThree2.count());
    if (otCount > 0)
      score += otCount * ScoreConfig::OPEN_THREE;
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
