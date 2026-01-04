#include "Evaluator.hpp"

#include <cmath>

int Evaluator::evaluate(const Board& board, Color aiColor) {
  // 防御重視のAIにするため、相手のスコア倍率を極端に上げる
  Color opponent = (aiColor == Color::BLACK) ? Color::WHITE : Color::BLACK;

  const BoardType& myStones =
      (aiColor == Color::BLACK) ? board.getBlackStones() : board.getWhiteStones();
  const BoardType& oppStones =
      (aiColor == Color::BLACK) ? board.getWhiteStones() : board.getBlackStones();
  BoardType empty = board.getEmptyStones();

  long long myScore = 0;
  long long oppScore = 0;

  // 1. 形（並び）の評価
  myScore += _countPatterns(myStones, empty);
  oppScore += _countPatterns(oppStones, empty);

  // 2. 捕獲数の評価
  int myCaptures = (aiColor == Color::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (aiColor == Color::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  myScore += myCaptures * ScoreConfig::CAPTURE;
  oppScore += oppCaptures * ScoreConfig::CAPTURE;

  // ★重要: 相手の攻撃に対する評価係数を 1.2 -> 5.0 に引き上げ
  // これにより「自分の良手」よりも「相手の妨害」を最優先するようになる
  return static_cast<int>(myScore - (oppScore * 5));
}

int Evaluator::evaluateMovePriority(const Board& board, int x, int y, Color color) {
  // 前回提示した「修正版 evaluateMovePriority」をそのまま使用してください
  // ただし、ScoreConfigの値が変わったので自動的に挙動が変わります

  int score = 0;
  const BoardType& myStones =
      (color == Color::BLACK) ? board.getBlackStones() : board.getWhiteStones();
  const BoardType& oppStones =
      (color == Color::BLACK) ? board.getWhiteStones() : board.getBlackStones();

  const int dx[] = {1, 0, 1, 1};
  const int dy[] = {0, 1, 1, -1};

  for (int i = 0; i < 4; ++i) {
    int countMy = 0;
    int countOpp = 0;

    // 範囲チェック (Radius 4)
    for (int k = 1; k <= 4; ++k) {
      // Positive
      int nx = x + dx[i] * k;
      int ny = y + dy[i] * k;
      if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
        int idx = ny * BOARD_WIDTH + nx;
        if (myStones.test(idx))
          countMy++;
        else if (oppStones.test(idx))
          countOpp++;
      }
      // Negative
      nx = x - dx[i] * k;
      ny = y - dy[i] * k;
      if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
        int idx = ny * BOARD_WIDTH + nx;
        if (myStones.test(idx))
          countMy++;
        else if (oppStones.test(idx))
          countOpp++;
      }
    }

    // --- 優先度判定 ---
    // 自分の攻撃
    if (countMy >= 4)
      score += ScoreConfig::PRIORITY_FIVE;
    else if (countMy == 3)
      score += ScoreConfig::PRIORITY_FOUR;
    else if (countMy == 2)
      score += ScoreConfig::PRIORITY_THREE;
    else if (countMy == 1)
      score += ScoreConfig::PRIORITY_TWO;

    // 相手の妨害（超・警戒モード）
    if (countOpp >= 4)
      score += ScoreConfig::PRIORITY_WIN_BLOCK;
    else if (countOpp == 3)
      score += ScoreConfig::PRIORITY_FOUR_BLOCK;
    else if (countOpp == 2)
      score += ScoreConfig::PRIORITY_THREE_BLOCK;
    else if (countOpp == 1)
      score += 100;
  }

  int centerDist = std::abs(x - 9) + std::abs(y - 9);
  score -= centerDist;
  return score;
}

int Evaluator::_countPatterns(const BoardType& stones, const BoardType& empty) {
  int score = 0;
  const int shifts[] = {1, BOARD_WIDTH, BOARD_WIDTH + 1, BOARD_WIDTH - 1};

  for (int s : shifts) {
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
    // 1. 五連 (XXXXX) - すでに勝っている状態（Minimax内での判定用）
    // ---------------------------------------------------------
    BoardType five = stones & s1 & s2 & s3 & s4;
    if (five.any())
      return ScoreConfig::WIN;

    // ---------------------------------------------------------
    // 2. Open Four (.XXXX.) - 止めないと負け
    // ---------------------------------------------------------
    // パターン: [空] [石] [石] [石] [石] [空]
    BoardType openFour = e0 & stones & s1 & s2 & s3 & e4;
    if (openFour.any())
      score += (int)openFour.count() * ScoreConfig::OPEN_FOUR;

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
