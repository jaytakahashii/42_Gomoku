#include "MenuScene.hpp"

MenuScene::MenuScene(sf::Font& font, const sf::Vector2u& initalSize)
    : _font(font),
      _titleText(font, "Gomoku"),
      _startButtonText(font, "START GAME"),
      _orderText(font, "Select: first or second?"),
      _firstText(font, "Go first"),
      _secondText(font, "Go second"),
      _easyText(font, "Easy"),
      _normalText(font, "Normal"),
      _hardText(font, "Hard"),
      _levelText(font, "Select the AI level."),
      _openingRuleText(font, "Select starting condition."),
      _standardText(font, "Standard"),
      _proText(font, "Pro"),
      _longProText(font, "Long Pro"),
      _tooltip(font) {
  this->_titleText.setCharacterSize(Theme::FontSize::Title);
  this->_titleText.setFillColor(Theme::Color::Text);
  this->_titleText.setStyle(sf::Text::Bold);
  sf::FloatRect titleBounds = this->_titleText.getLocalBounds();
  this->_titleText.setOrigin(titleBounds.getCenter());

  this->_startButton.setSize(Theme::Size::Button);
  this->_startButton.setFillColor(Theme::Color::ButtonIdle);
  this->_startButton.setOrigin(this->_startButton.getSize() / 2.f);

  this->_orderText.setCharacterSize(Theme::FontSize::Button);
  this->_orderText.setFillColor(sf::Color::Black);
  this->_orderText.setStyle(sf::Text::Bold);
  this->_orderText.setOrigin(this->_orderText.getLocalBounds().getCenter());

  this->_firstText.setCharacterSize(Theme::FontSize::Button);
  this->_firstText.setFillColor(sf::Color::Black);
  this->_firstText.setStyle(sf::Text::Bold);
  this->_firstText.setOrigin(this->_firstText.getLocalBounds().getCenter());

  this->_secondText.setCharacterSize(Theme::FontSize::Button);
  this->_secondText.setFillColor(sf::Color::Black);
  this->_secondText.setStyle(sf::Text::Bold);
  this->_secondText.setOrigin(this->_secondText.getLocalBounds().getCenter());

  this->_firstButton.setSize(Theme::Size::Button);
  this->_firstButton.setFillColor(Theme::Color::ButtonIdle);
  this->_firstButton.setOrigin(this->_firstButton.getSize() / 2.f);

  this->_secondButton.setSize(Theme::Size::Button);
  this->_secondButton.setFillColor(Theme::Color::ButtonIdle);
  this->_secondButton.setOrigin(this->_secondButton.getSize() / 2.f);

  this->_levelText.setCharacterSize(Theme::FontSize::Button);
  this->_levelText.setFillColor(sf::Color::Black);
  this->_levelText.setStyle(sf::Text::Bold);
  this->_levelText.setOrigin(this->_levelText.getLocalBounds().getCenter());

  this->_easyButton.setSize(Theme::Size::Button);
  this->_easyButton.setFillColor(Theme::Color::ButtonIdle);
  this->_easyButton.setOrigin(this->_easyButton.getSize() / 2.f);

  this->_easyText.setCharacterSize(Theme::FontSize::Button);
  this->_easyText.setFillColor(sf::Color::Black);
  this->_easyText.setStyle(sf::Text::Bold);
  this->_easyText.setOrigin(this->_easyText.getLocalBounds().getCenter());

  this->_normalButton.setSize(Theme::Size::Button);
  this->_normalButton.setFillColor(Theme::Color::ButtonIdle);
  this->_normalButton.setOrigin(this->_normalButton.getSize() / 2.f);

  this->_normalText.setCharacterSize(Theme::FontSize::Button);
  this->_normalText.setFillColor(sf::Color::Black);
  this->_normalText.setStyle(sf::Text::Bold);
  this->_normalText.setOrigin(_normalText.getLocalBounds().getCenter());

  this->_hardButton.setSize(Theme::Size::Button);
  this->_hardButton.setFillColor(Theme::Color::ButtonIdle);
  this->_hardButton.setOrigin(this->_hardButton.getSize() / 2.f);

  this->_hardText.setCharacterSize(Theme::FontSize::Button);
  this->_hardText.setFillColor(sf::Color::Black);
  this->_hardText.setStyle(sf::Text::Bold);
  this->_hardText.setOrigin(this->_hardText.getLocalBounds().getCenter());

  this->_openingRuleText.setCharacterSize(Theme::FontSize::Button);
  this->_openingRuleText.setFillColor(sf::Color::Black);
  this->_openingRuleText.setStyle(sf::Text::Bold);
  this->_openingRuleText.setOrigin(this->_openingRuleText.getLocalBounds().getCenter());

  this->_startButtonText.setCharacterSize(30);
  this->_startButtonText.setFillColor(sf::Color::White);
  sf::FloatRect btnTextBounds = this->_startButtonText.getLocalBounds();
  this->_startButtonText.setOrigin(btnTextBounds.getCenter());

  this->_standardButton.setSize(Theme::Size::Button);
  this->_standardButton.setFillColor(Theme::Color::ButtonIdle);
  this->_standardButton.setOrigin(this->_standardButton.getSize() / 2.f);

  this->_standardText.setCharacterSize(Theme::FontSize::Button);
  this->_standardText.setFillColor(sf::Color::Black);
  this->_standardText.setStyle(sf::Text::Bold);
  this->_standardText.setOrigin(this->_standardText.getLocalBounds().getCenter());

  this->_proButton.setSize(Theme::Size::Button);
  this->_proButton.setFillColor(Theme::Color::ButtonIdle);
  this->_proButton.setOrigin(this->_proButton.getSize() / 2.f);

  this->_proText.setCharacterSize(Theme::FontSize::Button);
  this->_proText.setFillColor(sf::Color::Black);
  this->_proText.setStyle(sf::Text::Bold);
  this->_proText.setOrigin(this->_proText.getLocalBounds().getCenter());

  this->_longProButton.setSize(Theme::Size::Button);
  this->_longProButton.setFillColor(Theme::Color::ButtonIdle);
  this->_longProButton.setOrigin(this->_proButton.getSize() / 2.f);

  this->_longProText.setCharacterSize(Theme::FontSize::Button);
  this->_longProText.setFillColor(sf::Color::Black);
  this->_longProText.setStyle(sf::Text::Bold);
  this->_longProText.setOrigin(this->_longProText.getLocalBounds().getCenter());

  onResize(initalSize);
}

void MenuScene::handleEvents(const EventList& events) {
  for (const auto e : events) {
    if (const auto* mousePtr = e->getIf<sf::Event::MouseButtonPressed>()) {
      if (mousePtr->button == sf::Mouse::Button::Left) {
        sf::Vector2f mousePos(static_cast<float>(mousePtr->position.x),
                              static_cast<float>(mousePtr->position.y));
        if (this->_startButton.getGlobalBounds().contains(mousePos)) {
          if (this->_onStartGame)
            this->_onStartGame(this->_turnOrder, this->_aiLevel, this->_openingRule);
        }

        if (this->_firstButton.getGlobalBounds().contains(mousePos)) {
          this->_turnOrder = TurnOrder::HumanFirst;
        } else if (this->_secondButton.getGlobalBounds().contains(mousePos)) {
          this->_turnOrder = TurnOrder::AIFirst;
        }

        if (this->_easyButton.getGlobalBounds().contains(mousePos)) {
          this->_aiLevel = AILevel::Easy;
        } else if (this->_normalButton.getGlobalBounds().contains(mousePos)) {
          this->_aiLevel = AILevel::Normal;
        } else if (this->_hardButton.getGlobalBounds().contains(mousePos)) {
          this->_aiLevel = AILevel::Hard;
        }

        if (this->_standardButton.getGlobalBounds().contains(mousePos)) {
          this->_openingRule = OpeningRule::Standard;
        } else if (this->_proButton.getGlobalBounds().contains(mousePos)) {
          this->_openingRule = OpeningRule::Pro;
        } else if (this->_longProButton.getGlobalBounds().contains(mousePos)) {
          this->_openingRule = OpeningRule::LongPro;
        }
      }
    }
  }
}

void MenuScene::update(float dt) {
  _updateButtonStatus(this->_firstButton, this->_turnOrder == TurnOrder::HumanFirst);
  _updateButtonStatus(this->_secondButton, this->_turnOrder == TurnOrder::AIFirst);

  _updateButtonStatus(this->_easyButton, _aiLevel == AILevel::Easy);
  _updateButtonStatus(this->_normalButton, _aiLevel == AILevel::Normal);
  _updateButtonStatus(this->_hardButton, _aiLevel == AILevel::Hard);

  _updateButtonStatus(this->_standardButton, this->_openingRule == OpeningRule::Standard);
  _updateButtonStatus(this->_proButton, this->_openingRule == OpeningRule::Pro);
  _updateButtonStatus(this->_longProButton, this->_openingRule == OpeningRule::LongPro);
}

void MenuScene::_checkHoverOnRules(sf::RenderWindow& window) {
  sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
  sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);
  bool isHovering = false;

  if (this->_standardButton.getGlobalBounds().contains(mousePos)) {
    _tooltip.show("No restrictions.", mousePos, window);
    isHovering = true;
  } else if (this->_proButton.getGlobalBounds().contains(mousePos)) {
    _tooltip.show("- First move must be center.\n- Third move must be outside 3x3 zone.", mousePos,
                  window);
    isHovering = true;
  } else if (this->_longProButton.getGlobalBounds().contains(mousePos)) {
    _tooltip.show("- First move must be center.\n- Third move must be outside 4x4 zone.", mousePos,
                  window);
    isHovering = true;
  }

  if (!isHovering) {
    _tooltip.hide();
  }
}

void MenuScene::_updateButtonStatus(sf::RectangleShape& button, bool isActive) {
  button.setFillColor(isActive ? Theme::Color::ButtonActive : Theme::Color::ButtonIdle);
}

void MenuScene::render(sf::RenderWindow& window) {
  window.clear(sf::Color(50, 50, 50));
  window.draw(this->_titleText);
  window.draw(this->_orderText);
  window.draw(this->_startButton);
  window.draw(this->_firstButton);
  window.draw(this->_firstText);
  window.draw(this->_secondButton);
  window.draw(this->_secondText);
  window.draw(this->_startButtonText);
  window.draw(this->_levelText);
  window.draw(this->_easyButton);
  window.draw(this->_easyText);
  window.draw(this->_normalButton);
  window.draw(this->_normalText);
  window.draw(this->_hardButton);
  window.draw(this->_hardText);
  window.draw(this->_openingRuleText);
  window.draw(this->_standardButton);
  window.draw(this->_standardText);
  window.draw(this->_proButton);
  window.draw(this->_proText);
  window.draw(this->_longProButton);
  window.draw(this->_longProText);
  _checkHoverOnRules(window);
  _tooltip.draw(window);
}

void MenuScene::onResize(const sf::Vector2u& windowSize) {
  float w = static_cast<float>(windowSize.x);
  float h = static_cast<float>(windowSize.y);

  this->_titleText.setPosition({w / 2.f, h * 0.3f});
  sf::Vector2f titlePos = this->_titleText.getPosition();
  this->_orderText.setPosition({titlePos.x, titlePos.y + 90.f});
  this->_startButton.setPosition({w / 2.f, h * 0.8f});
  this->_firstButton.setPosition(
      {this->_orderText.getPosition().x * 0.7f, this->_orderText.getPosition().y + 50.f});
  this->_secondButton.setPosition(
      {this->_orderText.getPosition().x * 1.3f, this->_orderText.getPosition().y + 50.f});
  this->_startButtonText.setPosition(this->_startButton.getPosition());
  this->_firstText.setPosition(this->_firstButton.getPosition());
  this->_secondText.setPosition(this->_secondButton.getPosition());
  this->_levelText.setPosition({w / 2.f, _secondButton.getPosition().y + 80});
  this->_easyButton.setPosition({w / 4.f, _levelText.getPosition().y + 50});
  this->_easyText.setPosition(_easyButton.getPosition());
  this->_normalButton.setPosition({w / 2.f, _levelText.getPosition().y + 50});
  this->_normalText.setPosition(_normalButton.getPosition());
  this->_hardButton.setPosition({(w / 4.f) * 3.f, _levelText.getPosition().y + 50});
  this->_hardText.setPosition(_hardButton.getPosition());
  this->_openingRuleText.setPosition({w / 2.f, _normalButton.getPosition().y + 80});
  this->_standardButton.setPosition({w / 4.f, _openingRuleText.getPosition().y + 50});
  this->_standardText.setPosition(_standardButton.getPosition());
  this->_proButton.setPosition({w / 2.f, _openingRuleText.getPosition().y + 50});
  this->_proText.setPosition(_proButton.getPosition());
  this->_longProButton.setPosition({(w / 4.f) * 3, _openingRuleText.getPosition().y + 50});
  this->_longProText.setPosition(_longProButton.getPosition());
}

void MenuScene::setOnStartGame(
    std::function<void(TurnOrder& turnOrder, AILevel& level, OpeningRule& rule)> callback) {
  this->_onStartGame = callback;
}
