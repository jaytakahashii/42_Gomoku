#include "AI.hpp"

#include <iostream>

Move AI::getBestMove(Board board, Player player) {
  _aiPlayer = player;

  // 本来は Iterative Deepening (深さ1, 2, 3...) を行うが、
  // まずは固定深さでテスト
  int depth = 3;

  Move bestMove = {-1, -1, -std::numeric_limits<int>::max()};

  // 候補手を取得
  std::vector<std::pair<int, int>> moves = _generateMoves(board);

  for (const std::pair<int, int>& p : moves) {
    int x = p.first;
    int y = p.second;

    // 1手進める (コピーを作成してシミュレーション)
    Board nextBoard = board;
    if (!nextBoard.makeMove(x, y))
      continue;

    // Minimax呼び出し (次は相手の番なので maximizing=false)
    int score = _minimax(nextBoard, depth - 1, -std::numeric_limits<int>::max(),
                         std::numeric_limits<int>::max(), false);

    if (score > bestMove.score) {
      bestMove.x = x;
      bestMove.y = y;
      bestMove.score = score;
    }
  }

  return bestMove;
}

int AI::_minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer) {
  // 1. 終局判定 or 深さ制限到達
  if (board.checkWin()) {
    // 勝ったプレイヤーがAIなら高得点、敵なら低得点
    // 深さが浅い(早い)勝ちほど価値が高いように depth を加算する
    return maximizingPlayer ? -_SCORE_WIN + depth : _SCORE_WIN - depth;
  }
  if (depth == 0) {
    return _evaluate(board, _aiPlayer);
  }

  auto moves = _generateMoves(board);

  if (maximizingPlayer) {
    int maxEval = -std::numeric_limits<int>::max();
    for (const auto& p : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(p.first, p.second))
        continue;

      int eval = _minimax(nextBoard, depth - 1, alpha, beta, false);
      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);
      if (beta <= alpha)
        break;  // Beta cut-off
    }
    return maxEval;
  } else {
    int minEval = std::numeric_limits<int>::max();
    for (const auto& p : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(p.first, p.second))
        continue;

      int eval = _minimax(nextBoard, depth - 1, alpha, beta, true);
      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);
      if (beta <= alpha)
        break;  // Alpha cut-off
    }
    return minEval;
  }
}

// 非常に単純な候補手生成
// (最適化のためには、石があるマスの周囲2マス以内のみを返すようにする)
std::vector<std::pair<int, int>> AI::_generateMoves(const Board& board) {
  std::vector<std::pair<int, int>> moves;
  std::bitset<MAX_CELLS> visited;  // 重複防止用

  // 盤面サイズ
  int size = BOARD_SIZE;

  // すべてのマスを走査するのではなく、
  // 「既に石が置かれている場所」を探し、その近傍を候補に追加する
  // ※ ビットボードなら、(black | white) のビットが立っている場所を取得し、
  //    その周囲のビットマスクと AND (NOT stones) を取ることで爆速化できますが、
  //    まずはループで実装します。

  bool isEmptyBoard = true;

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      if (board.getStoneAt(x, y) != Player::NONE) {
        isEmptyBoard = false;

        // 石がある場所(x,y)の周囲 radius=2 マスを探索候補に入れる
        int radius = 2;
        for (int dy = -radius; dy <= radius; ++dy) {
          for (int dx = -radius; dx <= radius; ++dx) {
            int nx = x + dx;
            int ny = y + dy;

            // 範囲外チェック
            if (nx < 0 || nx >= size || ny < 0 || ny >= size)
              continue;

            // 既に石がある場所は置けない
            if (board.getStoneAt(nx, ny) != Player::NONE)
              continue;

            int idx = ny * BOARD_WIDTH + nx;  // Boardクラスのindex計算に合わせる

            // まだ候補に入れていない場合のみ追加
            if (!visited.test(idx)) {
              visited.set(idx);
              moves.push_back({nx, ny});
            }
          }
        }
      }
    }
  }

  // 盤面が空の場合（初手）は、天元（中央）のみを返す
  if (isEmptyBoard) {
    moves.push_back({9, 9});
  }

  return moves;
}

// 評価関数 (仮)
// 今は「自分の石の数」を返すだけなどの超適当な実装でOK
int AI::_evaluate(const Board& board, Player player) {
  // 1. 自分の石、敵の石、空きマスのビットボードを用意
  // Boardクラスに getter を追加する必要があるかもしれません
  // (friend class AI にするか、public getterを使う)
  // ここでは public getter があると仮定して、Board内部の bitset を取得します。
  // ※ Board.hpp に `const std::bitset<MAX_CELLS>& getBlackStones() const` 等を追加してください。

  Player opponent = (player == Player::BLACK) ? Player::WHITE : Player::BLACK;

  // 自分の石、敵の石
  const BoardType& myStones =
      (player == Player::BLACK) ? board.getBlackStones() : board.getWhiteStones();
  const BoardType& oppStones =
      (player == Player::BLACK) ? board.getWhiteStones() : board.getBlackStones();

  // 空きマス (自分も敵もいない場所)
  // パディング(壁)部分は「石がある扱い」にしたいので、
  // 「自分でも敵でもない」=「真の空きマス」を計算します。
  // 壁の部分は myStonesにもoppStonesにも含まれないが、emptyとしても扱いたくない...
  // -> 壁は「敵」とみなして計算するのが安全です（Closed判定になるため）。
  // ここでは単純化のため、「石がない場所」をemptyとします。
  BoardType empty = ~(myStones | oppStones);

  int score = 0;

  // --- 攻撃スコア (自分の形) ---
  score += _countPatterns(myStones, empty);

  // --- 防御スコア (敵の形) ---
  // 敵に高い点数の形を作らせないことが重要
  // 敵のスコアを引く、あるいは「敵のOpenFourがある＝超危険」として処理
  // Minimax法なので、敵の手番での評価値は自然と考慮されますが、
  // ここで明示的に評価することも可能です。
  // 今回はシンプルに「自分のスコア - 敵のスコア」を返します。

  int oppScore = _countPatterns(oppStones, empty);

  // 捕獲数の評価
  int myCaptures = (player == Player::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (player == Player::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  score += myCaptures * _SCORE_CAPTURE;
  oppScore += oppCaptures * _SCORE_CAPTURE;

  return score - oppScore;
}

// パターンをカウントする関数
int AI::_countPatterns(const BoardType& stones, const BoardType& empty) {
  int score = 0;

  // 全方向 (横, 縦, 右下, 左下)
  const int shifts[] = {1, 20, 21, 19};

  for (int s : shifts) {
    // --- Open Four ( .XXXX. ) ---
    // パターン: [空] [石] [石] [石] [石] [空]
    // ビット演算: (e) & (s>>1) & (s>>2) & (s>>3) & (s>>4) & (e>>5)
    // ※右シフトで位置を合わせます
    BoardType s1 = stones >> s;
    BoardType s2 = s1 >> s;
    BoardType s3 = s2 >> s;
    BoardType s4 = s3 >> s;  // s >> 4*s
    BoardType e0 = empty;
    BoardType e5 = s4 >> s;  // empty >> 5*s

    BoardType openFour = e0 & s1 & s2 & s3 & s4 & e5;
    if (openFour.any()) {
      score += openFour.count() * _SCORE_OPEN_FOUR;
    }

    // --- Open Three ( .XXX. ) ---
    // パターン: [空] [石] [石] [石] [空]
    BoardType openThree = e0 & s1 & s2 & s3 & (s3 >> s);  // e >> 4*s
    if (openThree.any()) {
      score += openThree.count() * _SCORE_OPEN_THREE;
    }

    // --- Closed Four ( 2XXXX. or .XXXX2 ) ---
    // OpenFourでカウントしなかった「4連」を探す
    // 簡単のため「とにかく4連」を探して、OpenFour分を引く手抜き実装もアリですが、
    // ここでは「石が4つ連続している」ものを探します。
    BoardType four = s1 & s2 & s3 & s4;
    if (four.any()) {
      // OpenFourとしてカウント済みのものは重複するので考慮が必要
      // 単純加算だと二重計上になるので、
      // 「Fourの総数 - OpenFourの数」を ClosedFour とするロジックが良いです。
      int totalFours = four.count();
      int openFours = openFour.count();
      score += (totalFours - openFours) * _SCORE_CLOSED_FOUR;
    }
  }

  return score;
}
