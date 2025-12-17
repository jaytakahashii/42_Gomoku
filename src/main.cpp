#include <SFML/Graphics.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

const unsigned int BOARD_SIZE = 19;  // 19x19の盤面
const unsigned int CELL_SIZE = 40;
const float OFFSET = 20.0f;  // 盤面の端の余白
const unsigned int WINDOW_WIDTH = BOARD_SIZE * CELL_SIZE;
const unsigned int WINDOW_HEIGHT = WINDOW_WIDTH + 50;

enum class Player { None, Black, White };

class GomokuGame {
 public:
  sf::RenderWindow window;
  Player board[BOARD_SIZE][BOARD_SIZE];
  Player currentPlayer = Player::Black;
  float last_ai_time = 0.0f;

  GomokuGame() : window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "42 Gomoku AI") {
    // 盤面の初期化
    for (int i = 0; i < BOARD_SIZE; ++i)
      for (int j = 0; j < BOARD_SIZE; ++j)
        board[i][j] = Player::None;
  }

  void run() {
    while (window.isOpen()) {
      processEvents();
      render();
    }
  }

 private:
  void processEvents() {
    while (const std::optional event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window.close();
      } else if (const auto* mousePtr = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePtr->button == sf::Mouse::Button::Left) {
          handlePlayerClick(mousePtr->position.x, mousePtr->position.y);
        }
      }
    }
  }

  void handlePlayerClick(int x, int y) {
    // クリック座標を盤面のインデックスに変換（最も近い交点を探す）
    int col = std::round((x - OFFSET) / CELL_SIZE);
    int row = std::round((y - OFFSET) / CELL_SIZE);

    if (col >= 0 && col < (int)BOARD_SIZE && row >= 0 && row < (int)BOARD_SIZE) {
      if (board[row][col] == Player::None) {
        // 石を置く
        board[row][col] = currentPlayer;

        // 本来はここで「石の抜き(Capture)」や「三三禁止」のチェックを行う [cite: 22, 28]

        // ターン交代
        currentPlayer = (currentPlayer == Player::Black) ? Player::White : Player::Black;

        // AIのターンをシミュレート
        fakeAiMove();
      }
    }
  }

  void fakeAiMove() {
    auto start = std::chrono::high_resolution_clock::now();

    // --- ここでMin-Maxアルゴリズムを呼び出し、平均0.5秒以内に返答させる --- [cite: 39, 51]
    sf::sleep(sf::milliseconds(100));

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = end - start;
    last_ai_time = diff.count();
  }

  void render() {
    window.clear(sf::Color(200, 160, 100));

    // 1. 盤面の線を描画
    for (unsigned int i = 0; i < BOARD_SIZE; ++i) {
      float pos = (float)i * CELL_SIZE + OFFSET;
      sf::Vertex v_line[] = {sf::Vertex{{pos, OFFSET}},
                             sf::Vertex{{pos, (float)WINDOW_WIDTH - OFFSET}}};
      window.draw(v_line, 2, sf::PrimitiveType::Lines);
      sf::Vertex h_line[] = {sf::Vertex{{OFFSET, pos}},
                             sf::Vertex{{(float)WINDOW_WIDTH - OFFSET, pos}}};
      window.draw(h_line, 2, sf::PrimitiveType::Lines);
    }

    // 2. 石を描画
    for (int r = 0; r < BOARD_SIZE; ++r) {
      for (int c = 0; c < BOARD_SIZE; ++c) {
        if (board[r][c] != Player::None) {
          sf::CircleShape stone(15);
          stone.setOrigin({15, 15});
          stone.setPosition({c * (float)CELL_SIZE + OFFSET, r * (float)CELL_SIZE + OFFSET});
          stone.setFillColor(board[r][c] == Player::Black ? sf::Color::Black : sf::Color::White);
          window.draw(stone);
        }
      }
    }

    // 3. AIの思考時間を表示 [cite: 66, 68]
    sf::Font font;
    if (font.openFromFile("arial.ttf")) {
      sf::Text text(font, "AI Thinking Time: " + std::to_string(last_ai_time) + "s", 20);
      text.setFillColor(sf::Color::Black);
      text.setPosition({10.f, (float)WINDOW_WIDTH + 10.f});
      window.draw(text);
    }

    window.display();
  }
};

int main() {
  GomokuGame game;
  game.run();
  return 0;
}
