#include "Evaluator.hpp"

#include <cmath>

int Evaluator::evaluate(const Board& board, Color aiColor) {
  if (board.checkWinWithFive())
    return ScoreConfig::WIN;
  const BoardType& myStones = board.getMyStones(aiColor);
  const BoardType& oppStones = board.getOppStones(aiColor);
  BoardType empty = board.getEmptyStones();

  long long myScore = 0;
  long long oppScore = 0;

  // 1. 形（並び）の評価
  // 敵と自分でそれぞれ別の関数を使う
  myScore += _myCountPatterns(myStones, empty);
  oppScore += _oppCountPatterns(oppStones, empty);

  // 2. 捕獲数の評価
  int myCaptures = (aiColor == Color::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (aiColor == Color::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  myScore += myCaptures * ScoreConfig::CAPTURE;
  oppScore += oppCaptures * ScoreConfig::CAPTURE;

  // ★重要: 相手の攻撃に対する評価係数を 1.2 -> 5.0 に引き上げ
  // これにより「自分の良手」よりも「相手の妨害」を最優先するようになる
  return static_cast<int>(myScore - (oppScore * 5));
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

    // --- 2. パターンチェック (優先順位順) ---

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

int Evaluator::_myCountPatterns(const BoardType& stones, const BoardType& empty) {
  int score = 0;

  for (int s : ALL_DIRS) {
    // シフトしたビットボードを作成
    BoardType s1 = stones >> s;
    BoardType s2 = stones >> (2 * s);
    BoardType s3 = stones >> (3 * s);
    BoardType s4 = stones >> (4 * s);

    BoardType e1 = empty >> s;
    BoardType e2 = empty >> (2 * s);
    BoardType e3 = empty >> (3 * s);
    BoardType e4 = empty >> (4 * s);

    // 左端の空き
    BoardType e0 = empty;
    // 右端の空き (5つ先)
    BoardType e5 = empty >> (5 * s);

    // ---------------------------------------------------------
    // 1. Open Four (.XXXX.)
    // ---------------------------------------------------------
    // パターン: [空] [石] [石] [石] [石] [空]
    BoardType openFour = e0 & stones & s1 & s2 & s3 & e4;
    if (openFour.any())
      score += (int)openFour.count() * ScoreConfig::OPP_OPEN_FOUR;

    // ---------------------------------------------------------
    // 3. Closed Four / Broken Four (飛び四含む)
    // ---------------------------------------------------------
    // パターンA: XXXX. または .XXXX (端が空いていない)
    BoardType closedFour = (stones & s1 & s2 & s3) & (e0 | e4);  // 簡易判定

    // パターンB: Split Four (X.XXX, XX.XX, XXX.X)
    //  - X.XXX (石 空 石 石 石)
    BoardType splitFour1 = stones & e1 & s2 & s3 & s4;
    //  - XX.XX (石 石 空 石 石)
    BoardType splitFour2 = stones & s1 & e2 & s3 & s4;
    //  - XXX.X (石 石 石 空 石)
    BoardType splitFour3 = stones & s1 & s2 & e3 & s4;

    int cfCount =
        (int)(closedFour.count() + splitFour1.count() + splitFour2.count() + splitFour3.count());
    // OpenFourと重複している分は引く（厳密な計算より速度優先）
    if (cfCount > 0) {
      score += cfCount * ScoreConfig::CLOSED_FOUR;
    }

    // ---------------------------------------------------------
    // 4. Open Three (.XXX.)
    // ---------------------------------------------------------
    // パターン: .XXX.
    BoardType openThree = e0 & stones & s1 & s2 & e3;
    if (openThree.any())
      score += (int)openThree.count() * ScoreConfig::OPEN_THREE;

    // ---------------------------------------------------------
    // 5. Broken Three (.X.XX. / .XX.X.) - 飛び三
    // ---------------------------------------------------------
    // パターン: .X.XX.
    BoardType brokenThree1 = e0 & stones & e1 & s2 & s3 & e4;
    // パターン: .XX.X.
    BoardType brokenThree2 = e0 & stones & s1 & e2 & s3 & e4;

    if (brokenThree1.any())
      score += (int)brokenThree1.count() * ScoreConfig::OPEN_THREE;  // OpenThreeと同等の価値
    if (brokenThree2.any())
      score += (int)brokenThree2.count() * ScoreConfig::OPEN_THREE;

    // ---------------------------------------------------------
    // 6. Open Two (.XX.)
    // ---------------------------------------------------------
    // パターン: .XX.
    BoardType openTwo = e0 & stones & s1 & e2;
    if (openTwo.any())
      score += (int)openTwo.count() * ScoreConfig::OPEN_TWO;

    // ---------------------------------------------------------
    // 7. Broken Two / Gap Two (.X.X.)
    // ---------------------------------------------------------
    BoardType gapTwo = e0 & stones & e1 & s2 & e3;
    if (gapTwo.any())
      score += (int)gapTwo.count() * ScoreConfig::OPEN_TWO;
  }

  return score;
}

int Evaluator::_oppCountPatterns(const BoardType& stones, const BoardType& empty) {
  int score = 0;

  for (int s : ALL_DIRS) {
    // シフトしたビットボードを作成
    BoardType s1 = stones >> s;
    BoardType s2 = stones >> (2 * s);
    BoardType s3 = stones >> (3 * s);
    BoardType s4 = stones >> (4 * s);

    BoardType e1 = empty >> s;
    BoardType e2 = empty >> (2 * s);
    BoardType e3 = empty >> (3 * s);
    BoardType e4 = empty >> (4 * s);

    // 左端の空き
    BoardType e0 = empty;
    // 右端の空き (5つ先)
    BoardType e5 = empty >> (5 * s);

    // ---------------------------------------------------------
    // 1. Open Four (.XXXX.)
    // ---------------------------------------------------------
    // パターン: [空] [石] [石] [石] [石] [空]
    BoardType openFour = e0 & stones & s1 & s2 & s3 & e4;
    if (openFour.any())
      score += (int)openFour.count() * ScoreConfig::OPP_OPEN_FOUR;

    // ---------------------------------------------------------
    // 3. Closed Four / Broken Four (飛び四含む)
    // ---------------------------------------------------------
    // パターンA: XXXX. または .XXXX (端が空いていない)
    BoardType closedFour = (stones & s1 & s2 & s3) & (e0 | e4);  // 簡易判定

    // パターンB: Split Four (X.XXX, XX.XX, XXX.X)
    //  - X.XXX (石 空 石 石 石)
    BoardType splitFour1 = stones & e1 & s2 & s3 & s4;
    //  - XX.XX (石 石 空 石 石)
    BoardType splitFour2 = stones & s1 & e2 & s3 & s4;
    //  - XXX.X (石 石 石 空 石)
    BoardType splitFour3 = stones & s1 & s2 & e3 & s4;

    int cfCount =
        (int)(closedFour.count() + splitFour1.count() + splitFour2.count() + splitFour3.count());
    // OpenFourと重複している分は引く（厳密な計算より速度優先）
    if (cfCount > 0) {
      score += cfCount * ScoreConfig::CLOSED_FOUR;
    }

    // ---------------------------------------------------------
    // 4. Open Three (.XXX.)
    // ---------------------------------------------------------
    // パターン: .XXX.
    BoardType openThree = e0 & stones & s1 & s2 & e3;
    if (openThree.any())
      score += (int)openThree.count() * ScoreConfig::OPEN_THREE;

    // ---------------------------------------------------------
    // 5. Broken Three (.X.XX. / .XX.X.) - 飛び三
    // ---------------------------------------------------------
    // パターン: .X.XX.
    BoardType brokenThree1 = e0 & stones & e1 & s2 & s3 & e4;
    // パターン: .XX.X.
    BoardType brokenThree2 = e0 & stones & s1 & e2 & s3 & e4;

    if (brokenThree1.any())
      score += (int)brokenThree1.count() * ScoreConfig::OPEN_THREE;  // OpenThreeと同等の価値
    if (brokenThree2.any())
      score += (int)brokenThree2.count() * ScoreConfig::OPEN_THREE;

    // ---------------------------------------------------------
    // 6. Open Two (.XX.)
    // ---------------------------------------------------------
    // パターン: .XX.
    BoardType openTwo = e0 & stones & s1 & e2;
    if (openTwo.any())
      score += (int)openTwo.count() * ScoreConfig::OPEN_TWO;

    // ---------------------------------------------------------
    // 7. Broken Two / Gap Two (.X.X.)
    // ---------------------------------------------------------
    BoardType gapTwo = e0 & stones & e1 & s2 & e3;
    if (gapTwo.any())
      score += (int)gapTwo.count() * ScoreConfig::OPEN_TWO;
  }

  return score;
}
