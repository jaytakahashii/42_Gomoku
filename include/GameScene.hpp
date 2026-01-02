#ifndef GAMESCENE_HPP
#define GAMESCENE_HPP

#include <Board.hpp>
#include <Enums.hpp>
#include <Scene.hpp>
#include <Theme.hpp>
#include <functional>
#include <future>
#include <iomanip>
#include <list>
#include <sstream>
#include <thread>

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
  void setOnEsc(std::function<void()> callback);

  void reset();

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
  void _handleMessage(float df);
  std::function<void(const std::string& winner)> _onGameOver;
  sf::Text _countWhiteCaptures;
  sf::Text _countBlackCaptures;
  sf::Text _turnNotification;
  float _turnAnimTimer = 0.0f;
  void _notifyPlayerTurn(float df);

  float _aiMoveTimer = 0.0f;
  std::future<Move> _aiFuture;
  bool _isAIThinking = false;
  void _applyAIMove(Move move);
  sf::Text _aiInfoText;
  sf::Clock _aiClock;
  void _handleAIProcess(float df);

  sf::RectangleShape _undoButton;
  sf::Text _undoText;
  void _onUndo();
  void _updateCaptures();

  sf::RectangleShape _aiAssistButton;
  sf::Text _aiAssistText;
  std::future<Move> _hintFuture;
  bool _isCalculatingHint = false;
  sf::Vector2i _hintMove = {-1, -1};
  void _onAIAssist();
  void _handleHint();
  sf::CircleShape _makeHintCircle() const;

  std::function<void()> _onEsc;
};

#endif
