#include "AI.hpp"

#include <iostream>

Move AI::getBestMove(const Board& board, Color color, AILevel level) {
  // 1. セットアップ
  Board clone = board;  // 盤面のクローンを作成 (1回のみ)
  _aiPlayer = color;
  int maxDepth = _getDepthFromLevel(level);

  std::cout << "AI Thinking... (Depth: " << maxDepth << ")" << std::endl;

  // 2. 手の生成（1回のみ実行）
  std::vector<Move> moves = _generateMoves(clone);

  // 打つ場所がない場合（引き分けや盤面埋まり）
  if (moves.empty()) {
    return {-1, -1, 0};
  }

  if (moves.size() == 1) {
    return moves[0];
  }

  Move bestMove = {-1, -1, std::numeric_limits<int>::min()};
  int alpha = std::numeric_limits<int>::min();
  int beta = std::numeric_limits<int>::max();

  // 3. ルートノード探索 (Minimaxの開始点)
  // 反復深化のループは削除し、いきなり maxDepth で探索します
  for (const Move& m : moves) {
    // 手を打つ
    if (!clone.makeMove(m.x, m.y))
      continue;

    // 次は相手の番なので maximizingPlayer = false, 深さは -1
    int score = _minimax(clone, maxDepth - 1, alpha, beta, false);

    // 手を戻す
    clone.undo();

    // 最善手の更新
    if (score > bestMove.score) {
      bestMove = m;
      bestMove.score = score;

      // デバッグ表示（現在の一番良い手）
      // std::cout << "Candidate: (" << m.x << "," << m.y << ") Score: " << score << std::endl;
    }

    // Alpha値の更新
    alpha = std::max(alpha, score);

    // // ルートノードでのBetaカット（理論上は発生しないが、必勝手が見つかったら打ち切るなど）
    // if (score >= ScoreConfig::WIN - 1000) {
    //   break;  // 勝ち確定ならこれ以上探さない
    // }
  }

  return bestMove;
}

int AI::_minimax(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
  // 1. 終局判定
  if (board.checkWin()) {
    // 自分が勝ったなら高得点。残り深さが大きい（早い勝ち）ほど高得点。
    return maximizingPlayer ? -(ScoreConfig::WIN + depth) : (ScoreConfig::WIN + depth);
  }

  // 2. 葉ノード（指定深さに到達）
  if (depth == 0) {
    return Evaluator::evaluate(board, _aiPlayer);
  }

  // 3. 手の生成
  std::vector<Move> moves = _generateMoves(board);
  if (moves.empty())
    return 0;  // 引き分け

  // 4. 再帰探索
  if (maximizingPlayer) {
    int maxEval = std::numeric_limits<int>::min();
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      int eval = _minimax(board, depth - 1, alpha, beta, false);

      board.undo();

      maxEval = std::max(maxEval, eval);
      alpha = std::max(alpha, eval);

      // Beta Cut-off
      if (beta <= alpha)
        break;
    }
    return maxEval;
  } else {
    int minEval = std::numeric_limits<int>::max();
    for (const Move& m : moves) {
      if (!board.makeMove(m.x, m.y))
        continue;

      int eval = _minimax(board, depth - 1, alpha, beta, true);

      board.undo();

      minEval = std::min(minEval, eval);
      beta = std::min(beta, eval);

      // Alpha Cut-off
      if (beta <= alpha)
        break;
    }
    return minEval;
  }
}

std::vector<Move> AI::_generateMoves(const Board& board) {
  std::vector<Move> moves;

  BoardType empty = board.getEmptyStones();
  BoardType occupied = board.getOccupiedStones();

  // 1. 初手（中央）
  if (occupied.none()) {
    moves.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2, 0});
    return moves;
  }

  // 2. 探索範囲の決定（石の周囲2マス）
  BoardType candidates;
  const int W = BOARD_WIDTH;
  const int shifts[] = {1, -1, W, -W, W + 1, -(W + 1), W - 1, -(W - 1)};

  BoardType mask = occupied;
  for (int s : shifts) {
    BoardType s1 = (s > 0) ? (occupied << s) : (occupied >> -s);
    BoardType s2 = (s > 0) ? (occupied << (s * 2)) : (occupied >> (-s * 2));
    mask |= s1 | s2;
  }
  candidates = mask & empty;

  // 3. 候補手の評価とリスト化
  bool urgentMoveFound = false;  // 負け確定を防ぐ手があるか

  for (int i = 0; i < MAX_CELLS; ++i) {
    if (candidates.test(i)) {
      int y = i / W;
      int x = i % W;
      if (x >= BOARD_SIZE)
        continue;

      int priority = Evaluator::evaluateMovePriority(board, x, y, board.getCurrentTurn());

      // ★修正ポイント1: 緊急事態の検知
      // PRIORITY_WIN_BLOCK (50,000,000) 以上のスコアは「相手の4」を防ぐ手
      if (priority >= ScoreConfig::PRIORITY_WIN_BLOCK) {
        urgentMoveFound = true;
      }

      moves.push_back({x, y, priority});
    }
  }

  // 4. ソート (スコアが高い順)
  if (moves.empty())
    return moves;

  std::sort(moves.begin(), moves.end(),
            [](const Move& a, const Move& b) { return a.score > b.score; });

  // ★修正ポイント2: 必殺の「緊急手フィルタリング」
  // もし「これを打たないと負ける」という手があるなら、それ以外の手は探索するだけ時間の無駄なので全て捨てる。
  // これにより、読みの深さが実質無限大になり、絶対に見落とさなくなる。
  if (urgentMoveFound) {
    std::vector<Move> urgentMoves;
    for (const auto& m : moves) {
      if (m.score >= ScoreConfig::PRIORITY_WIN_BLOCK) {
        urgentMoves.push_back(m);
      }
    }
    return urgentMoves;  // 4連止めのみを返す
  }

  // ★修正ポイント3: ビームサーチの改良
  // 上位N手に絞るが、「相手の3連を防ぐ手」などの準・緊急手は、
  // たとえ20位以下であっても絶対に捨ててはいけない。

  std::vector<Move> finalMoves;
  int count = 0;
  // 相手の3連を防ぐレベルのスコア閾値 (Evaluatorに合わせて調整)
  const int SEMI_URGENT_THRESHOLD = ScoreConfig::PRIORITY_FOUR_BLOCK;

  for (const auto& m : moves) {
    // 上位20手以内、または「準・緊急手（相手の3を止める）」なら採用
    if (count < 20 || m.score >= SEMI_URGENT_THRESHOLD) {
      finalMoves.push_back(m);
      count++;
    }
  }

  return finalMoves;
}

int AI::_getDepthFromLevel(AILevel level) {
  switch (level) {
    case AILevel::Easy:
      return DEPTH_EASY;
    case AILevel::Medium:
      return DEPTH_NORMAL;
    case AILevel::Hard:
      return DEPTH_HARD;
    default:
      return DEPTH_HARD;
  }
}
