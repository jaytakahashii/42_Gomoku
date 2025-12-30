#ifndef GAMESCENE_HPP
#define GAMESCENE_HPP

#include <Board.hpp>
#include <Enums.hpp>
#include <Scene.hpp>
#include <Theme.hpp>
#include <functional>
#include <list>

#include "AI.hpp"

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

  void setOnGameOver(std::function<void(const std::string& winner)> callback);
  void setAILevel(AILevel& level);
  void setTurnOrder(TurnOrder turnOrder);

 private:
  struct FloatingMessage {
    sf::Text text;
    float timer;

    FloatingMessage(const sf::Font& font, const std::string& str, sf::Vector2f pos);
  };

  sf::Font& _font;
  std::list<FloatingMessage> _activeMessages;
  const unsigned int _boardSize = BOARD_SIZE;
  const unsigned int _cellSize = CELL_SIZE;
  sf::Vector2f _boardOffset;
  AILevel _aiLevel;

  Board _board;
  void handleClick(int x, int y);
  void displayTimedMessage(const std::string& message, sf::Vector2f pos);
  std::function<void(const std::string& winner)> _onGameOver;
  sf::Text _countWhiteCaptures;
  sf::Text _countBlackCaptures;
  sf::Text _turnNotification;
  float _turnAnimTimer = 0.0f;
};

#endif
