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

// ビット演算を用いた高速パターンマッチング
// 重複カウント（Open4をClosed4としても数える等）を避ける処理を入れています
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

// Move Ordering用の評価
// 配列確保を避けて高速化
int Evaluator::evaluateMovePriority(const Board& board, int x, int y, Color myColor) {
  int score = 0;
  Color oppColor = (myColor == Color::BLACK) ? Color::WHITE : Color::BLACK;

  // 中央に近いほど加点（基本戦術）
  // int centerDist = std::abs(x - 9) + std::abs(y - 9);
  // score += (10 - centerDist) * 10;

  // 捕獲手のボーナス (Capture is usually good)
  // ここで実装するには「この手を打つと捕獲が発生するか」のチェックが必要
  // 重くなるのでAIクラスでのgenerateMoves時にフラグを渡すか、簡易的なら省略

  // 4方向チェック
  // [1,0], [0,1], [1,1], [1,-1]
  score += _CheckLineScore(board, x, y, 1, 0, myColor, oppColor);
  score += _CheckLineScore(board, x, y, 0, 1, myColor, oppColor);
  score += _CheckLineScore(board, x, y, 1, 1, myColor, oppColor);
  score += _CheckLineScore(board, x, y, 1, -1, myColor, oppColor);

  return score;
}

int Evaluator::_CheckLineScore(const Board& board, int x, int y, int dx, int dy, Color myColor,
                               Color oppColor) {
  int score = 0;

  // 自分の石として置いた場合の並び
  int myConsecutive = 1;
  int myOpenEnds = 0;

  // 相手の石が置いてあった場合の並び（＝相手の妨害）
  int oppConsecutive = 0;
  int oppOpenEnds = 0;

  // --- 自分の攻撃力チェック ---
  // 正方向
  for (int k = 1; k <= 5; ++k) {
    int nx = x + dx * k, ny = y + dy * k;
    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
      break;
    Color c = board.getColorAt(nx, ny);
    if (c == myColor)
      myConsecutive++;
    else {
      if (c == Color::NONE)
        myOpenEnds++;
      break;
    }
  }
  // 逆方向
  for (int k = 1; k <= 5; ++k) {
    int nx = x - dx * k, ny = y - dy * k;
    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
      break;
    Color c = board.getColorAt(nx, ny);
    if (c == myColor)
      myConsecutive++;
    else {
      if (c == Color::NONE)
        myOpenEnds++;
      break;
    }
  }

  // --- 相手の攻撃阻止チェック ---
  // もしここが相手の石だったら、何連になっていたか？
  // 正方向
  for (int k = 1; k <= 5; ++k) {
    int nx = x + dx * k, ny = y + dy * k;
    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
      break;
    Color c = board.getColorAt(nx, ny);
    if (c == oppColor)
      oppConsecutive++;
    else {
      if (c == Color::NONE)
        oppOpenEnds++;
      break;
    }
  }
  // 逆方向
  for (int k = 1; k <= 5; ++k) {
    int nx = x - dx * k, ny = y - dy * k;
    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
      break;
    Color c = board.getColorAt(nx, ny);
    if (c == oppColor)
      oppConsecutive++;
    else {
      if (c == Color::NONE)
        oppOpenEnds++;
      break;
    }
  }
  // (自分も相手も含めて)中心は1つなので +1 補正して考える必要があるが
  // oppConsecutiveは「中心を除いた隣接数」としてカウントしているため、
  // 「ここに打てば相手のN連を止めた」ことになる。
  // 例: X . X -> 間に打つ -> left=1, right=1 -> oppConsecutive=2. Total 3.

  // --- スコアリング ---

  // 1. 自分の勝ち (5連)
  if (myConsecutive >= 5)
    score += ScoreConfig::PRIORITY_WIN;

  // 2. 相手の勝ち阻止 (相手が既に4つ並んでいる、または飛び4)
  // oppConsecutive が 4以上なら、既に5連ができているので遅い（ありえない状況だが）
  // oppConsecutive == 3 (つまり . X X X . の真ん中や端) -> 次に4になるのを防ぐ
  // ★重要: 五目並べでは「4連」を作られた時点でほぼ負け。
  // 相手が「3連（飛び含む）」を持っていて、ここが「4つ目」になる場所なら、全力で阻止。
  if (oppConsecutive >= 4)
    score += ScoreConfig::PRIORITY_BLOCK_WIN;  // 相手の5連阻止

  // 3. 自分のOpen 4
  if (myConsecutive == 4) {
    if (myOpenEnds >= 2)
      score += ScoreConfig::PRIORITY_OPEN_FOUR;
    else
      score += ScoreConfig::PRIORITY_OPEN_THREE;  // Closed 4 is weaker but good
  }

  // 4. 相手のOpen 4 阻止 (相手が3連を持っていて、両端が空いている場所)
  if (oppConsecutive == 3) {
    if (oppOpenEnds >= 2)
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_4;  // 相手のOpen4阻止
    else
      score += ScoreConfig::PRIORITY_BLOCK_OPEN_3;
  }

  // 5. 自分のOpen 3
  if (myConsecutive == 3 && myOpenEnds >= 2)
    score += ScoreConfig::PRIORITY_OPEN_THREE;

  return score;
}
