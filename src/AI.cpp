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
// 次のステップでここを強化します
int AI::_evaluate(const Board& board, Player player) {
  // TODO: パターンマッチングによる本格的な評価

  // 仮実装: ランダムに動かないように、中心に近いほど高得点にする
  int score = 0;
  // ...
  return score;
}
