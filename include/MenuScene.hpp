#pragma once

#include <functional>

#include "Enums.hpp"
#include "Scene.hpp"
#include "Theme.hpp"
#include "Tooltip.hpp"

class MenuScene : public Scene {
 public:
  MenuScene(sf::Font& font, const sf::Vector2u& initialSize);

  void handleEvents(const EventList& events);
  void update(float dt);
  void render(sf::RenderWindow& window);
  void onResize(const sf::Vector2u& windowSize);

  void setOnStartGame(
      std::function<void(TurnOrder& turnOrder, AILevel& level, OpeningRule& rule, bool isPvP)>
          callback);

 private:
  sf::Font& _font;

  void _initTitle();
  sf::Text _titleText;

  void _initStartButton();
  sf::RectangleShape _startButton;
  sf::Text _startButtonText;

  void _initOrderButtons();
  sf::Text _orderText;
  sf::RectangleShape _firstButton;
  sf::RectangleShape _secondButton;
  sf::Text _firstText;
  sf::Text _secondText;

  void _initAILevelButtons();
  sf::RectangleShape _easyButton;
  sf::RectangleShape _normalButton;
  sf::RectangleShape _hardButton;
  sf::Text _levelText;
  sf::Text _normalText;
  sf::Text _easyText;
  sf::Text _hardText;

  TurnOrder _turnOrder = TurnOrder::HumanFirst;
  AILevel _aiLevel = AILevel::Normal;
  std::function<void(TurnOrder& turnOrder, AILevel& level, OpeningRule& rule, bool isPvP)>
      _onStartGame;

  void _initVSButtons();
  sf::RectangleShape _pvpButton;
  sf::RectangleShape _pvAIButton;
  sf::Text _pvpText;
  sf::Text _pvAIText;
  bool _isPvP = true;

  OpeningRule _openingRule = OpeningRule::Standard;
  void _initOpeningRuleButtons();
  sf::Text _openingRuleText;
  sf::RectangleShape _standardButton;
  sf::Text _standardText;
  sf::RectangleShape _proButton;
  sf::Text _proText;
  sf::RectangleShape _longProButton;
  sf::Text _longProText;
  Tooltip _tooltip;

  void _checkHoverOnRules(sf::RenderWindow& window);

  void _updateButtonStatus(sf::RectangleShape& button, bool isActive);
};
