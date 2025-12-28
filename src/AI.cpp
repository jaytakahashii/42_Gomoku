#include "AI.hpp"

#include <iostream>

// 時間制限 (ミリ秒) - 安全マージンを取って450msくらいにする
const int TIME_LIMIT_MS = 1000;

bool AI::_isTimeUp() {
  auto now = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime).count();
  return duration >= TIME_LIMIT_MS;
}

Move AI::getBestMove(Board board, Player player) {
  _aiPlayer = player;
  _startTime = std::chrono::high_resolution_clock::now();
  _timeOut = false;

  Move bestMove = {-1, -1, -std::numeric_limits<int>::max()};

  // 反復深化: 深さ1, 2, 3... と増やしていく
  // 最大深さはとりあえず20にしておく（時間切れで止まるのでOK）
  for (int depth = 1; depth <= 20; ++depth) {
    // この深さでの探索を実行
    // 戻り値をMove構造体ごと返すように _minimax を少し改造するか、
    // あるいはここ（ルートノード）だけ特別扱いしてループする

    int alpha = -std::numeric_limits<int>::max();
    int beta = std::numeric_limits<int>::max();
    Move currentDepthBest = {-1, -1, -std::numeric_limits<int>::max()};

    // 候補手を取得 (ソート済み)
    std::vector<Move> moves = _generateMoves(board);
    if (moves.empty())
      break;

    for (const Move& m : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(m.x, m.y))
        continue;

      // 再帰呼び出し
      int score = _minimax(nextBoard, depth - 1, alpha, beta, false);

      // 時間切れチェック: 探索途中で時間が来たら、この深さの結果は信頼できないので捨てる
      if (_timeOut)
        break;

      if (score > currentDepthBest.score) {
        currentDepthBest.x = m.x;
        currentDepthBest.y = m.y;
        currentDepthBest.score = score;
      }
      // Alpha更新
      alpha = std::max(alpha, score);
    }

    if (_timeOut) {
      std::cout << "Time up at depth " << depth << std::endl;
      break;
    }

    // 時間内に探索完了できたら、その結果をベストとして採用
    bestMove = currentDepthBest;
    std::cout << "Depth " << depth << " finished. Best: " << bestMove.score << std::endl;

    // もし「必勝（これ以上探さなくていい）」が見つかったら終了
    if (bestMove.score >= _SCORE_WIN - 100)
      break;
  }

  return bestMove;
}

int AI::_minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer) {
  if (_isTimeUp()) {
    _timeOut = true;
    return 0;  // 値はどうでもいい
  }

  if (_timeOut)
    return 0;  // 既にタイムアウトしていれば即帰る

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
      if (!nextBoard.makeMove(p.x, p.y))
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
      if (!nextBoard.makeMove(p.x, p.y))
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

// 簡易評価用のヘルパー (Move Ordering用)
// ちゃんと計算すると重いので、「石の近く」や「攻撃に参加できるか」だけ軽く見る
int AI::_evaluateMoveOrdering(const Board& board, int x, int y, Player player) {
  // 実際に置いてみて、評価関数(深さ0)を呼ぶ
  // ※ 本来はもっと軽量な計算が良いが、まずはこれで精度を出す
  Board next = board;
  if (!next.makeMove(x, y))
    return -10000000;  // 禁じ手などは論外

  // _evaluate は「その盤面の静的評価」を返す
  return _evaluate(next, player);
}

// 非常に単純な候補手生成
// (最適化のためには、石があるマスの周囲2マス以内のみを返すようにする)
std::vector<Move> AI::_generateMoves(const Board& board) {
  std::vector<Move> moves;
  BoardType visited;  // 重複防止用

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

              // Move構造体を作ってスコアを入れる
              Move m;
              m.x = nx;
              m.y = ny;
              // ここで「並び替えのためのスコア」を計算
              // AIの手番での評価値を出したいので _aiPlayer を渡す
              // m.score = _evaluateMoveOrdering(board, nx, ny, _aiPlayer);
              m.score = _evaluateMoveOrdering(board, nx, ny, board.getCurrentTurn());
              moves.push_back(m);  // , m.score
            }
          }
        }
      }
    }
  }

  if (isEmptyBoard) {
    // 盤面が空の場合、中央に置くのが最善手
    int center = size / 2;
    moves.push_back({center, center, 0});
  }
  // ★重要: スコアが高い順にソートする (降順)
  // これにより Alpha-Beta が効率的に枝刈りできる
  std::sort(moves.begin(), moves.end(),
            [](const Move& a, const Move& b) { return a.score > b.score; });

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

  return score - (oppScore * 1.5);
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
    BoardType e4 = empty >> (4 * s);
    BoardType e5 = s4 >> s;  // empty >> 5*s

    BoardType openFour = e0 & s1 & s2 & s3 & s4 & e5;
    if (openFour.any()) {
      score += openFour.count() * _SCORE_OPEN_FOUR;
    }

    // --- Open Three ( .XXX. ) ---
    // パターン: [空] [石] [石] [石] [空]
    BoardType openThree = e0 & s1 & s2 & s3 & e4;
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
