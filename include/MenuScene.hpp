#pragma once

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

  void setOnStartGame(
      std::function<void(TurnOrder& turnOrder, AILevel& level, OpeningRule& rule)> callback);

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
  std::function<void(TurnOrder& turnOrder, AILevel& level, OpeningRule& rule)> _onStartGame;

  OpeningRule _openingRule = OpeningRule::Standard;
  sf::Text _openingRuleText;

  sf::RectangleShape _standardButton;
  sf::Text _standardText;
  sf::RectangleShape _proButton;
  sf::Text _proText;
  sf::RectangleShape _longProButton;
  sf::Text _longProText;

  void _updateButtonStatus(sf::RectangleShape& button, bool isActive);
};
