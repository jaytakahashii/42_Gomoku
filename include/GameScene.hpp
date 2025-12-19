#ifndef GAMESCENE_HPP
#define GAMESCENE_HPP

#include <Scene.hpp>
#include <functional>

const unsigned int BOARD_SIZE = 19;
const unsigned int CELL_SIZE = 40;
const float OFFSET = 20.0f;
const unsigned int WINDOW_WIDTH = BOARD_SIZE * CELL_SIZE;
const unsigned int WINDOW_HEIGHT = WINDOW_WIDTH + 50;

class GameScene : public Scene {
 public:
  GameScene(sf::Font& font, const sf::Vector2u& initialSize);
  void handleEvents(const EventList& events);
  void update(float dt);
  void render(sf::RenderWindow& window);
  void onResize(const sf::Vector2u& windowSize);

  void setOnGameOver(std::function<void()> callback);

 private:
  sf::Font& _font;
  const unsigned int _boardSize = BOARD_SIZE;
  const unsigned int _cellSize = CELL_SIZE;
  sf::Vector2f _boardOffset;

  std::bitset<BOARD_SIZE*(BOARD_SIZE + 1)> _blackStones;
  std::bitset<BOARD_SIZE*(BOARD_SIZE + 1)> _whiteStones;
  void handleClick(int x, int y);
  std::function<void()> _onGameOver;

  bool _isBlackTurn = false;  // 仮
};

#endif
