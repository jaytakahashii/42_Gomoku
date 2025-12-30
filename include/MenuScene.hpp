#ifndef MENUSCENE_HPP
#define MENUSCENE_HPP

#include <Enums.hpp>
#include <Scene.hpp>
#include <Theme.hpp>
#include <functional>

class MenuScene : public Scene {
 public:
  MenuScene(sf::Font& font, const sf::Vector2u& initialSize);

  void handleEvents(const EventList& events);
  void update(float dt);
  void render(sf::RenderWindow& window);
  void onResize(const sf::Vector2u& windowSize);

  void setOnStartGame(std::function<void(TurnOrder turnOrder, AILevel& level)> callback);

 private:
  sf::Font& _font;
  sf::Text _titleText;
  sf::RectangleShape _startButton;
  sf::Text _orderText;
  sf::RectangleShape _firstButton;
  sf::RectangleShape _secondButton;
  sf::RectangleShape _easyButton;
  sf::RectangleShape _mediumButton;
  sf::RectangleShape _hardButton;
  sf::Text _levelText;
  sf::Text _mediumText;
  sf::Text _easyText;
  sf::Text _hardText;
  sf::Text _firstText;
  sf::Text _secondText;
  sf::Text _startButtonText;
  TurnOrder _turnOrder = TurnOrder::HumanFirst;
  AILevel _aiLevel = AILevel::Medium;
  std::function<void(TurnOrder turnOrder, AILevel& level)> _onStartGame;
};

#endif
