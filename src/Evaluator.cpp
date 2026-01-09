#include "Evaluator.hpp"

#include <cmath>
#include <iostream>

int Evaluator::evaluate(const Board& board, Color aiColor) {
  if (board.checkWinColor(aiColor)) {
    return ScoreConfig::WIN;
  }

  const BoardType& myStones = board.getMyStones(aiColor);
  const BoardType& oppStones = board.getOppStones(aiColor);
  BoardType empty = board.getEmptyStones();

  long long myScore = 0;
  long long oppScore = 0;

  // 1. 形（並び）の評価
  // 敵と自分でそれぞれ別の関数を使う
  myScore += _CountPatterns(myStones, empty, oppStones);
  oppScore += _CountPatterns(oppStones, empty, myStones);

  // 2. 捕獲数の評価
  int myCaptures = (aiColor == Color::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (aiColor == Color::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  myScore += myCaptures * ScoreConfig::CAPTURE;
  oppScore += oppCaptures * ScoreConfig::CAPTURE;

  // 相手の攻撃に対する評価係数を引き上げ
  // これにより「自分の良手」よりも「相手の妨害」を最優先するようになる
  return static_cast<int>(myScore - (oppScore * 1.5));
}

/**
 * 手の優先順位評価点数を返す
 * Arguments:
 * - board: 現在の盤面
 * - x, y: 評価する手の座標
 * - myColor: 評価する側の色
 * - oppColor: 相手の色
 */
int checkPatternScore(const Board& board, int x, int y, Color myColor) {
  int maxScore = 0;

  const BoardType& myStones = board.getMyStones(myColor);
  const BoardType& oppStones = board.getOppStones(myColor);

  // 4方向
  const int dx[] = {1, 0, 1, 1};
  const int dy[] = {0, 1, 1, -1};

  for (int i = 0; i < 4; ++i) {
    // --- 1. 配列への展開 (Radius 4) ---
    // 0:空, 1:自分, 2:相手, 3:壁
    int line[9];
    int center = 4;  // line[4]が現在地(x,y)

    // 中心には自分の石を置くと仮定
    line[center] = 1;

    for (int k = -4; k <= 4; ++k) {
      if (k == 0)
        continue;
      int nx = x + dx[i] * k;
      int ny = y + dy[i] * k;

      if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) {
        line[center + k] = 3;  // 壁
      } else {
        int idx = ny * BOARD_WIDTH + nx;
        if (myStones.test(idx))
          line[center + k] = 1;
        else if (oppStones.test(idx))
          line[center + k] = 2;
        else
          line[center + k] = 0;
      }
    }

    // --- 2. Patternチェック (優先順位順) ---

    // [Priority S+] 相手の4連
    int leftOpp = 0, rightOpp = 0;
    for (int k = 1; k <= 4 && line[center - k] == 2; ++k)
      leftOpp++;
    for (int k = 1; k <= 4 && line[center + k] == 2; ++k)
      rightOpp++;

    if (leftOpp + rightOpp >= 4) {
      return ScoreConfig::PRIORITY_MAYBE_OPP_WIN;
    }

    // 相手の3連
    if (leftOpp + rightOpp == 3) {
      // [Priority A+] Open 3
      bool leftOpen = (line[center - leftOpp - 1] == 0);
      bool rightOpen = (line[center + rightOpp + 1] == 0);
      if (leftOpen && rightOpen) {
        if (maxScore < ScoreConfig::PRIORITY_OPP_OPEN_THREE)
          maxScore = ScoreConfig::PRIORITY_OPP_OPEN_THREE;
      }
      // [Priority C] Closed 3
      else if (leftOpen || rightOpen) {
        if (maxScore < ScoreConfig::PRIORITY_OPP_CLOSED_THREE)
          maxScore = ScoreConfig::PRIORITY_OPP_CLOSED_THREE;
      }
    }

    // [Priority B] Capture
    // 1221 の形
    bool capture = false;
    if (line[center - 1] == 2 && line[center - 2] == 2 && line[center - 3] == 1)
      capture = true;
    if (line[center + 1] == 2 && line[center + 2] == 2 && line[center + 3] == 1)
      capture = true;

    if (capture) {
      int capScore = ScoreConfig::PRIORITY_CAPTURE;
      if (maxScore < capScore)
        maxScore = capScore;
    }

    // 自分の攻撃 (Open 4 / Open 3)

    // [Priority A-] 自分の5連
    // 11111
    int consecutive = 0;
    for (int k = 0; k < 9; ++k) {
      if (line[k] == 1)
        consecutive++;
      else
        consecutive = 0;
      if (consecutive >= 5) {
        if (maxScore < ScoreConfig::PRIORITY_MAYBE_MY_WIN)
          maxScore = ScoreConfig::PRIORITY_MAYBE_MY_WIN;
      }
    }

    // 自分の4連・3連
    int leftMy = 0, rightMy = 0;
    for (int k = 1; k <= 4 && line[center - k] == 1; ++k)
      leftMy++;
    for (int k = 1; k <= 4 && line[center + k] == 1; ++k)
      rightMy++;
    int myLen = leftMy + rightMy + 1;

    if (myLen == 4) {  // 4連になる
      bool leftOpen = (line[center - leftMy - 1] == 0);
      bool rightOpen = (line[center + rightMy + 1] == 0);

      // [Priority B] Open 4
      if (leftOpen && rightOpen) {
        if (maxScore < ScoreConfig::PRIORITY_MY_OPEN_FOUR)
          maxScore = ScoreConfig::PRIORITY_MY_OPEN_FOUR;

        // [Priority C] Closed 4
      } else if (leftOpen || rightOpen) {
        if (maxScore < ScoreConfig::PRIORITY_MY_CLOSED_FOUR)
          maxScore = ScoreConfig::PRIORITY_MY_CLOSED_FOUR;
      }

      // [Priority C] Open 3
    } else if (myLen == 3) {
      bool leftOpen = (line[center - leftMy - 1] == 0);
      bool rightOpen = (line[center + rightMy + 1] == 0);
      if (leftOpen && rightOpen) {
        if (maxScore < ScoreConfig::PRIORITY_MY_OPEN_THREE)
          maxScore = ScoreConfig::PRIORITY_MY_OPEN_THREE;
      }
    }
  }

  return maxScore;
}

/**
 * 手の優先度を簡易評価する
 * Arguments:
 * - board: 現在の盤面 (ReadOnly)
 * - x, y: 評価する手の座標
 * - color: 手を打つプレイヤーの色
 */
int Evaluator::evaluateMovePriority(const Board& board, int x, int y, Color color) {
  int score = checkPatternScore(board, x, y, color);

  // 戦術的加点（中央重視）
  int centerDist = std::abs(x - 9) + std::abs(y - 9);
  score -= centerDist;

  return score;
}

int Evaluator::_CountPatterns(const BoardType& myBoard, const BoardType& empty,
                              const BoardType& oppBoard) {
  int score = 0;

  for (int s : ALL_DIRS) {
    // 自分の石のシフト（s1は1つ先、s2は2つ先...）
    BoardType s1 = myBoard >> s;
    BoardType s2 = myBoard >> (2 * s);
    BoardType s3 = myBoard >> (3 * s);
    BoardType s4 = myBoard >> (4 * s);

    // 空点のシフト
    BoardType e0 = empty;
    BoardType e1 = empty >> s;
    BoardType e2 = empty >> (2 * s);
    BoardType e3 = empty >> (3 * s);
    BoardType e4 = empty >> (4 * s);
    BoardType e5 = empty >> (5 * s);

    // 敵の石のシフト（Capture判定用）
    BoardType o1 = oppBoard >> s;
    BoardType o2 = oppBoard >> (2 * s);

    // ---------------------------------------------------------
    // 1. Priority S+: Open Four (.XXXX.)
    // ---------------------------------------------------------
    // Pattern: .XXXX.
    BoardType openFour = e0 & s1 & s2 & s3 & s4 & e5;
    if (openFour.any())
      score += (int)openFour.count() * ScoreConfig::OPEN_FOUR;

    // ---------------------------------------------------------
    // 3. Priority S Closed Four / Broken Four
    // ---------------------------------------------------------
    // Pattern: X X X X .  (基準は最初のX)
    // 条件: myBoard(X) & s1(X) & s2(X) & s3(X) & e4(.)
    // ※ e0は含めない（自分自身がいるのでe0はfalseになる）
    BoardType closedFour1 = myBoard & s1 & s2 & s3 & e4;

    // Pattern: . X X X X (基準は最初の.)
    // e0 & s1 & s2 & s3 & s4
    // ※ openFour で数えたものは除外したいが、簡易的には重複しても良い（スコア調整でカバー）
    BoardType closedFour2 = e0 & s1 & s2 & s3 & s4;

    // Pattern: Split Four (飛び四)
    // X . X X X (基準: X)
    BoardType splitFour1 = myBoard & e1 & s2 & s3 & s4;
    // X X . X X (基準: X)
    BoardType splitFour2 = myBoard & s1 & e2 & s3 & s4;
    // X X X . X (基準: X)
    BoardType splitFour3 = myBoard & s1 & s2 & e3 & s4;

    int cfCount = (int)(closedFour1.count() + closedFour2.count() + splitFour1.count() +
                        splitFour2.count() + splitFour3.count());

    // OpenFourの分を引く（簡易処理：OpenFourは closedFour2 にもマッチしてしまうため）
    if (openFour.any()) {
      cfCount -= (int)openFour.count();
    }

    if (cfCount > 0) {
      score += cfCount * ScoreConfig::CLOSED_FOUR;
    }

    // ---------------------------------------------------------
    // 4. Priority A: Open Three (.XXX.)
    // ---------------------------------------------------------
    // Pattern: . X X X . (基準: 左の.)
    BoardType openThree = e0 & s1 & s2 & s3 & e4;

    // Pattern: . X . X X . (飛び三 A)
    BoardType brokenThree1 = e0 & s1 & e2 & s3 & s4 & e5;

    // Pattern: . X X . X . (飛び三 B)
    BoardType brokenThree2 = e0 & s1 & s2 & e3 & s4 & e5;

    if (openThree.any())
      score += (int)openThree.count() * ScoreConfig::OPEN_THREE;
    if (brokenThree1.any())
      score += (int)brokenThree1.count() * ScoreConfig::OPEN_THREE;
    if (brokenThree2.any())
      score += (int)brokenThree2.count() * ScoreConfig::OPEN_THREE;

    // ---------------------------------------------------------
    // 5. Priority B: Open Two (.XX.)
    // ---------------------------------------------------------
    // Pattern: .XX.
    BoardType openTwo = e0 & myBoard & s1 & e2;
    // Pattern: .X.X.
    BoardType gapTwo = e0 & myBoard & e1 & s2 & e3;

    if (openTwo.any())
      score += (int)openTwo.count() * ScoreConfig::OPEN_TWO;
    if (gapTwo.any())
      score += (int)gapTwo.count() * ScoreConfig::OPEN_TWO;
  }

  return score;
}
