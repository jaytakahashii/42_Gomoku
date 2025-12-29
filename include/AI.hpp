#ifndef AI_HPP
#define AI_HPP

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <vector>

#include "Board.hpp"

struct Move {
  int x;
  int y;
  int score;
};

class AI {
 public:
  AI() = default;

  // 制限時間内に最善手を計算して返す
  Move getBestMove(Board board, Player player);

 private:
  // --- 定数定義 ---

  // 評価スコア
  static constexpr int _SCORE_WIN = 100000000;
  static constexpr int _SCORE_OPEN_FOUR = 10000000;  // 次に確実に勝てる形
  static constexpr int _SCORE_CAPTURE = 1000000;     // 石を取る価値
  static constexpr int _SCORE_CLOSED_FOUR = 100000;  // 防がれないと勝てる形
  static constexpr int _SCORE_OPEN_THREE = 100000;   // 次にOpenFourになる形

  // 探索設定
  static constexpr int TIME_LIMIT_MS = 1000;  // 思考時間（ミリ秒）
  static constexpr int MAX_DEPTH = 20;        // 反復深化の最大深さ

  // --- メンバ変数 ---

  Player _aiPlayer;  // AIの手番（色）
  bool _timeOut;     // 時間切れフラグ
  std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;

  // --- 探索・思考ロジック ---

  // 時間切れかどうかを判定する
  bool _isTimeUp();

  // Minimax法（Alpha-Beta法）による再帰探索
  int _minimax(Board board, int depth, int alpha, int beta, bool maximizingPlayer);

  // --- 手の生成・順序付け ---

  // 有効な手を生成し、有望な順にソートして返す
  std::vector<Move> _generateMoves(const Board& board);

  // 手の並び替え（Move Ordering）のための簡易評価
  // 軽い処理で「良さそうな手」を高く評価する
  int _evaluatePoint(const Board& board, int x, int y, Player player);

  // --- 盤面評価 ---

  // 盤面全体の静的な評価値を計算する（深さ0の時に呼ぶ）
  int _evaluate(const Board& board, Player player);

  // 特定のパターンの個数を数えてスコア化するヘルパー
  int _countPatterns(const BoardType& stones, const BoardType& empty);
};

#endif
