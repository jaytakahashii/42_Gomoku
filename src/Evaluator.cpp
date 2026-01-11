#include "Evaluator.hpp"

#include <algorithm>
#include <iostream>

int Evaluator::evaluate(const Board& board, Color aiColor) {
  // 1. 勝利確定盤面のチェック
  // checkWinは「直前に打った人」の勝利判定なので、ここではcheckWinColorを使用
  if (board.checkWin(aiColor))
    return ScoreConfig::WIN;
  Color oppColor = (aiColor == Color::BLACK) ? Color::WHITE : Color::BLACK;
  if (board.checkWin(oppColor))
    return -ScoreConfig::WIN;

  const BoardType& myStones = board.getMyStones(aiColor);
  const BoardType& oppStones = board.getOppStones(aiColor);
  BoardType empty = board.getEmptyStones();

  BoardType myDeadStones = board.getCapturableStones(aiColor);
  BoardType oppDeadStones = board.getCapturableStones(oppColor);

  BoardType mySafeStones = myStones & ~myDeadStones;
  BoardType oppSafeStones = oppStones & ~oppDeadStones;

  long long myScore = 0;
  long long oppScore = 0;

  // 2. パターン評価 (自分の形 vs 敵の形)
  myScore += _CountPatterns(mySafeStones, empty);
  oppScore += _CountPatterns(oppSafeStones, empty);

  // 3. 捕獲状態の評価
  // あと少しで勝てる（9捕獲など）場合はスコアを跳ね上げる
  int myCaptures = (aiColor == Color::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (aiColor == Color::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  myScore += myCaptures * ScoreConfig::CAPTURE_SCORE;
  oppScore += oppCaptures * ScoreConfig::CAPTURE_SCORE;

  // 捕獲リーチの評価 (あと1ペアで勝ちならOpenFour並の脅威)
  if (myCaptures >= 8)
    myScore += ScoreConfig::OPEN_FOUR;
  if (oppCaptures >= 8)
    oppScore += ScoreConfig::OPEN_FOUR;

  // 4. 捕獲脅威（Threat）の評価
  // 「次に取れる/取られる」ペアの数に基づく加点・減点
  // 1ペア取られる = 2石失う + 相手に点が入る。非常に痛い。
  // count() は石の数なので、ペア数にするには / 2 する
  if (myDeadStones.any()) {
    // 自分の石が狙われている -> 大幅減点（防がせるため）
    // OPEN_THREE以上のペナルティを与えて、防御を優先させる
    myScore -=
        (myDeadStones.count() / 2) * (ScoreConfig::CAPTURE_SCORE * 2 + ScoreConfig::OPEN_THREE);
  }

  if (oppDeadStones.any()) {
    // 相手の石を狙える -> 加点（攻撃のチャンス）
    // ただし、「勝てる手(5連)」より優先しないよう控えめに
    myScore += (oppDeadStones.count() / 2) * (ScoreConfig::CAPTURE_SCORE + ScoreConfig::OPEN_TWO);
  }

  // 相手のスコアを少し重く見る（防御的AI）
  return static_cast<int>(myScore - (oppScore * 1.2));
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
