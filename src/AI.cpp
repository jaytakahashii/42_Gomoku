#include "AI.hpp"

#include <iostream>

// -------------------------------------------------------------------------
// 公開メソッド (Public Methods)
// -------------------------------------------------------------------------

Move AI::getBestMove(Board board, Color color, AILevel level) {
  _aiPlayer = color;
  _startTime = std::chrono::high_resolution_clock::now();
  _timeOut = false;

  Move bestMove = {-1, -1, -std::numeric_limits<int>::max()};

  // 反復深化探索 (Iterative Deepening)
  // 深さ1, 2, 3... と徐々に深く読み、時間切れになったら直前の深さの結果を採用する
  for (int depth = 1; depth <= MAX_DEPTH; ++depth) {
    // ルートノード（最初の分岐）の処理
    // ここでMove構造体を取得し、時間切れチェックを行う
    int alpha = -std::numeric_limits<int>::max();
    int beta = std::numeric_limits<int>::max();
    Move currentDepthBest = {-1, -1, -std::numeric_limits<int>::max()};

    std::vector<Move> moves = _generateMoves(board);
    if (moves.empty()) {
      break;
    }

    // ルートでの探索ループ
    for (const Move& m : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(m.x, m.y)) {
        continue;
      }

      // 次の手番は相手（Min層）なので maximizingPlayer=false
      int score = _minimax(nextBoard, depth - 1, alpha, beta, false);

      // 探索中に時間が切れた場合、この深さの結果は不完全なので破棄する
      if (_timeOut) {
        break;
      }

      if (score > currentDepthBest.score) {
        currentDepthBest.x = m.x;
        currentDepthBest.y = m.y;
        currentDepthBest.score = score;
      }

      // Alpha値の更新（より良い手が見つかった）
      alpha = std::max(alpha, score);
    }

    if (_timeOut) {
      std::cout << "Time up at depth " << depth << std::endl;
      break;
    }

    // 最後まで探索できた場合のみ、最善手を更新
    bestMove = currentDepthBest;
    std::cout << "Depth " << depth << " finished. Best Score: " << bestMove.score << std::endl;

    // 必勝が見つかったらこれ以上深く読む必要はない
    if (bestMove.score >= _SCORE_WIN - 100) {
      break;
    }
  }

  return bestMove;
}

// -------------------------------------------------------------------------
// 探索ロジック (Search Logic)
// -------------------------------------------------------------------------

bool AI::_isTimeUp() {
  std::chrono::time_point<std::chrono::high_resolution_clock> now =
      std::chrono::high_resolution_clock::now();

  long long duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime).count();

  // 安全マージンとして少し早めに切り上げる (例: 950ms)
  return duration >= (TIME_LIMIT_MS - 50);
}

int AI::_minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer) {
  // 時間切れチェック（重くなりすぎないよう、このチェックは重要）
  if (_timeOut || _isTimeUp()) {
    _timeOut = true;
    return 0;  // 値は無視されるので適当な値を返す
  }

  // 1. 終局判定
  if (board.checkWin()) {
    // AIが勝ちならプラス、負けならマイナス
    // 浅い階層（早いターン）での勝ちほど価値を高くする (+ depth)
    return maximizingPlayer ? -_SCORE_WIN + depth : _SCORE_WIN - depth;
  }

  // 2. 深さ制限到達（葉ノード）
  if (depth == 0) {
    return _evaluate(board, _aiPlayer);
  }

  // 3. 次の手の生成
  std::vector<Move> moves = _generateMoves(board);
  if (moves.empty()) {
    // 打つ場所がない（引き分け）
    return 0;
  }

  // 4. 再帰探索 (Alpha-Beta Pruning)
  if (maximizingPlayer) {
    int maxEval = -std::numeric_limits<int>::max();
    for (const Move& m : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(m.x, m.y))
        continue;

      int eval = _minimax(nextBoard, depth - 1, alpha, beta, false);

      // 時間切れならループを抜ける
      if (_timeOut)
        return 0;

      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);

      // Beta Cut-off: これ以上探しても相手が選ばない手なので打ち切り
      if (beta <= alpha) {
        break;
      }
    }
    return maxEval;
  } else {
    int minEval = std::numeric_limits<int>::max();
    for (const Move& m : moves) {
      Board nextBoard = board;
      if (!nextBoard.makeMove(m.x, m.y))
        continue;

      int eval = _minimax(nextBoard, depth - 1, alpha, beta, true);

      if (_timeOut)
        return 0;

      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);

      // Alpha Cut-off: これ以上探しても自分が選ばない手なので打ち切り
      if (beta <= alpha) {
        break;
      }
    }
    return minEval;
  }
}

// -------------------------------------------------------------------------
// 手の生成と順序付け (Move Generation & Ordering)
// -------------------------------------------------------------------------

std::vector<Move> AI::_generateMoves(const Board& board) {
  std::vector<Move> moves;
  BoardType visited;  // 重複追加を防ぐためのビットセット

  bool isEmptyBoard = true;
  int size = BOARD_SIZE;
  int radius = 2;  // 石の周囲何マスを探索候補にするか

  // 盤面上のすべての石を走査し、その周囲の空きマスを候補に追加する
  // 本来はビット演算で高速化すべきだが、可読性のためループ処理とする
  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      if (board.getPlayerAt(x, y) != Player::NONE) {
        isEmptyBoard = false;

        // 石がある場所(x,y)の近傍を探索
        for (int dy = -radius; dy <= radius; ++dy) {
          for (int dx = -radius; dx <= radius; ++dx) {
            int nx = x + dx;
            int ny = y + dy;

            // 範囲外または既に石がある場所はスキップ
            if (nx < 0 || nx >= size || ny < 0 || ny >= size)
              continue;
            if (board.getPlayerAt(nx, ny) != Player::NONE)
              continue;

            int idx = ny * BOARD_WIDTH + nx;

            // まだ候補リストに入れていない場合のみ追加
            if (!visited.test(idx)) {
              visited.set(idx);

              Move m;
              m.x = nx;
              m.y = ny;
              // Alpha-Beta法の効率化のため、この時点で簡易スコアをつける
              m.score = _evaluatePoint(board, nx, ny, board.getCurrentTurn());
              moves.push_back(m);
            }
          }
        }
      }
    }
  }

  // 初手（盤面が空）の場合は中央に打つ
  if (isEmptyBoard) {
    int center = size / 2;
    moves.push_back({center, center, 0});
    return moves;
  }

  // 重要: スコアが高い順にソートする (Move Ordering)
  // これにより、良い手から先に探索され、枝刈りが多く発生するようになる
  std::sort(moves.begin(), moves.end(),
            [](const Move& a, const Move& b) { return a.score > b.score; });

  // ビームサーチ的な枝刈り: 上位の手だけを残す
  // 候補手が多すぎると深く読めないため、有望な15手程度に絞る
  const size_t MAX_MOVES_TO_CHECK = 15;
  if (moves.size() > MAX_MOVES_TO_CHECK) {
    moves.resize(MAX_MOVES_TO_CHECK);
  }

  return moves;
}

int AI::_evaluatePoint(const Board& board, int x, int y, Color color) {
  int score = 0;

  // 自分と相手の石を取得
  const BoardType& myStones =
      (color == Color::BLACK) ? board.getBlackStones() : board.getWhiteStones();
  const BoardType& oppStones =
      (color == Color::BLACK) ? board.getWhiteStones() : board.getBlackStones();

  // 4方向（横、縦、右下、左下）
  const int dx[] = {1, 0, 1, 1};
  const int dy[] = {0, 1, 1, -1};

  // 置こうとしている場所(x,y)を中心に、4方向の繋がり具合を見る
  for (int i = 0; i < 4; ++i) {
    int countMy = 1;   // 今置いた石
    int countOpp = 0;  // 敵に挟まれているか

    // 前後4マスを確認
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
          break;  // 敵にブロックされている
        } else {
          break;  // 空きマス（伸びしろ）
        }
      }
    }

    // 簡易スコア付け
    if (countMy >= 5)
      score += 100000;
    else if (countMy == 4)
      score += 10000;
    else if (countMy == 3)
      score += 1000;
    else if (countMy == 2)
      score += 100;

    // 敵の石が近くにある＝防御の手としての価値を加算
    if (countOpp > 0)
      score += 50;
  }

  // 中央に近いほど少し評価を高くする（戦術的価値）
  int centerDist = std::abs(x - 9) + std::abs(y - 9);
  score -= centerDist;

  return score;
}

// -------------------------------------------------------------------------
// 評価関数 (Evaluation)
// -------------------------------------------------------------------------

int AI::_evaluate(const Board& board, Color color) {
  // 評価は常に「自分のスコア - 敵のスコア」で行う
  Color opponent = (color == Color::BLACK) ? Color::WHITE : Color::BLACK;

  const BoardType& myStones =
      (color == Color::BLACK) ? board.getBlackStones() : board.getWhiteStones();
  const BoardType& oppStones =
      (color == Color::BLACK) ? board.getWhiteStones() : board.getBlackStones();

  // 空きマス（自分も敵もいない場所）のビットマップを作成
  // 壁の部分を含まないよう注意が必要だが、ここでは簡易的に「どちらの石もない場所」とする
  BoardType empty = ~(myStones | oppStones);

  int myScore = 0;
  int oppScore = 0;

  // 形の評価（連の数）
  myScore += _countPatterns(myStones, empty);
  oppScore += _countPatterns(oppStones, empty);

  // 捕獲数の評価
  int myCaptures = (color == Color::BLACK) ? board.getBlackCaptures() : board.getWhiteCaptures();
  int oppCaptures = (color == Color::BLACK) ? board.getWhiteCaptures() : board.getBlackCaptures();

  myScore += myCaptures * _SCORE_CAPTURE;
  oppScore += oppCaptures * _SCORE_CAPTURE;

  // 敵のスコアは重めに引く（攻撃よりも防御を優先させるため）
  return myScore - static_cast<int>(oppScore * 1.5);
}

int AI::_countPatterns(const BoardType& stones, const BoardType& empty) {
  int score = 0;

  // 4方向のシフト量 (横1, 縦20, 斜め21, 斜め19)
  // Boardの仕様に依存する値
  const int shifts[] = {1, BOARD_WIDTH, BOARD_WIDTH + 1, BOARD_WIDTH - 1};

  for (int s : shifts) {
    // ビットシフトを利用してパターンマッチングを行う
    // stones >> s は「sだけずらした位置に自分の石があるか」を表す

    BoardType s1 = stones >> s;
    BoardType s2 = s1 >> s;
    BoardType s3 = s2 >> s;
    BoardType s4 = s3 >> s;

    // 空きマスのチェック用
    BoardType e0 = empty;             // 左端
    BoardType e4 = empty >> (4 * s);  // 4つ右
    BoardType e5 = s4 >> s;           // 5つ右

    // --- Open Four ( .XXXX. ) ---
    // パターン: [空] [石] [石] [石] [石] [空]
    BoardType openFour = e0 & s1 & s2 & s3 & s4 & e5;
    if (openFour.any()) {
      score += (int)openFour.count() * _SCORE_OPEN_FOUR;
    }

    // --- Open Three ( .XXX. ) ---
    // パターン: [空] [石] [石] [石] [空]
    // 注: e4 は 4*s シフト済み
    BoardType openThree = e0 & s1 & s2 & s3 & e4;
    if (openThree.any()) {
      score += (int)openThree.count() * _SCORE_OPEN_THREE;
    }

    // --- Closed Four / Four ( XXXX ) ---
    // OpenFourも含んでしまうため、後で差し引く処理が必要
    BoardType four = s1 & s2 & s3 & s4;
    if (four.any()) {
      int totalFours = (int)four.count();
      int openFours = (int)openFour.count();

      // 純粋なClosed Four (端が塞がれている4連) のみを加算
      score += (totalFours - openFours) * _SCORE_CLOSED_FOUR;
    }
  }

  return score;
}
