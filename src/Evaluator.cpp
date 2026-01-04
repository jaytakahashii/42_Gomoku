#include "Evaluator.hpp"

#include <cmath>  // for std::abs

int Evaluator::evaluate(const Board& board, Color aiColor) {
  // 評価は常に「自分のスコア - 敵のスコア」
  // AIにとって良い盤面ほどプラス、悪いほどマイナスになるようにする
  Color opponent = (aiColor == Color::BLACK) ? Color::WHITE : Color::BLACK;

  const BoardType& myStones =
      (aiColor == Color::BLACK) ? board.getBlackStones() : board.getWhiteStones();
  const BoardType& oppStones =
      (aiColor == Color::BLACK) ? board.getWhiteStones() : board.getBlackStones();

  // 空きマス（簡易的に、両者の石がない場所）
  BoardType empty = board.getEmptyStones();

  int myScore = 0;
  int oppScore = 0;

  // 1. 形（並び）の評価
  myScore += _countPatterns(myStones, empty);
  oppScore += _countPatterns(oppStones, empty);

  // 2. 捕獲数の評価
  int myCaptures = (aiColor == Color::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (aiColor == Color::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  myScore += myCaptures * ScoreConfig::CAPTURE;
  oppScore += oppCaptures * ScoreConfig::CAPTURE;

  // 防御を重視するため、敵のスコア係数を少し高く設定（1.2倍〜1.5倍など）
  return myScore - static_cast<int>(oppScore * 1.2);
}

int Evaluator::evaluateMovePriority(const Board& board, int x, int y, Color color) {
  int score = 0;

  // 自分と相手の石
  const BoardType& myStones =
      (color == Color::BLACK) ? board.getBlackStones() : board.getWhiteStones();
  const BoardType& oppStones =
      (color == Color::BLACK) ? board.getWhiteStones() : board.getBlackStones();

  // 4方向（横、縦、右下、左下）
  const int dx[] = {1, 0, 1, 1};
  const int dy[] = {0, 1, 1, -1};

  // 置こうとしている場所を中心に連続性をチェック
  for (int i = 0; i < 4; ++i) {
    int countMy = 1;   // 今置く石
    int countOpp = 0;  // 敵に挟まれているか等のチェック用

    for (int sign = -1; sign <= 1; sign += 2) {
      for (int k = 1; k <= 4; ++k) {
        int nx = x + dx[i] * k * sign;
        int ny = y + dy[i] * k * sign;

        if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
          break;

        int idx = ny * BOARD_WIDTH + nx;
        if (myStones.test(idx)) {
          countMy++;
        } else if (oppStones.test(idx)) {
          countOpp++;
          break;  // 敵がいる
        } else {
          break;  // 空きマス
        }
      }
    }

    // 簡易重み付け
    if (countMy >= 5)
      score += ScoreConfig::PRIORITY_FIVE;
    else if (countMy == 4)
      score += ScoreConfig::PRIORITY_FOUR;
    else if (countMy == 3)
      score += ScoreConfig::PRIORITY_THREE;
    else if (countMy == 2)
      score += ScoreConfig::PRIORITY_TWO;

    // 敵の近くは防御価値あり
    if (countOpp > 0)
      score += 50;
  }

  // 戦術的価値：中央に近いほど少し加点
  int centerDist = std::abs(x - 9) + std::abs(y - 9);
  score -= centerDist;

  return score;
}

int Evaluator::_countPatterns(const BoardType& stones, const BoardType& empty) {
  int score = 0;

  // 方向ごとのシフト量
  const int shifts[] = {1, BOARD_WIDTH, BOARD_WIDTH + 1, BOARD_WIDTH - 1};

  for (int s : shifts) {
    BoardType s1 = stones >> s;
    BoardType s2 = s1 >> s;
    BoardType s3 = s2 >> s;
    BoardType s4 = s3 >> s;

    // 空きマスチェック用
    BoardType e0 = empty;             // 左端
    BoardType e4 = empty >> (4 * s);  // 4つ右
    BoardType e5 = s4 >> s;           // 5つ右

    // --- Open Four ( .XXXX. ) ---
    BoardType openFour = e0 & s1 & s2 & s3 & s4 & e5;
    if (openFour.any()) {
      score += (int)openFour.count() * ScoreConfig::OPEN_FOUR;
    }

    // --- Open Three ( .XXX. ) ---
    // 注: e4 は 4*s シフト済み
    BoardType openThree = e0 & s1 & s2 & s3 & e4;
    if (openThree.any()) {
      score += (int)openThree.count() * ScoreConfig::OPEN_THREE;
    }

    // --- Closed Four / Four ( XXXX ) ---
    BoardType four = s1 & s2 & s3 & s4;
    if (four.any()) {
      int totalFours = (int)four.count();
      int openFours = (int)openFour.count();
      // 純粋なClosed Fourのみ加算
      score += (totalFours - openFours) * ScoreConfig::CLOSED_FOUR;
    }
  }

  return score;
}
