#include <MenuScene.hpp>

MenuScene::MenuScene(sf::Font& font, const sf::Vector2u& initalSize)
    : _font(font),
      _titleText(font, "Gomoku"),
      _startButtonText(font, "START GAME"),
      _orderText(font, "You can choose: first or second?"),
      _firstText(font, "Go first"),
      _secondText(font, "Go second"),
      _easyText(font, "Easy"),
      _mediumText(font, "Medium"),
      _hardText(font, "Hard"),
      _levelText(font, "You can select the AI level.") {
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
  this->_orderText.setOrigin(_orderText.getLocalBounds().getCenter());

  this->_firstText.setCharacterSize(Theme::FontSize::Button);
  this->_firstText.setFillColor(sf::Color::Black);
  this->_firstText.setStyle(sf::Text::Bold);
  this->_firstText.setOrigin(_firstText.getLocalBounds().getCenter());

  this->_secondText.setCharacterSize(Theme::FontSize::Button);
  this->_secondText.setFillColor(sf::Color::Black);
  this->_secondText.setStyle(sf::Text::Bold);
  this->_secondText.setOrigin(_secondText.getLocalBounds().getCenter());

  this->_firstButton.setSize(Theme::Size::Button);
  this->_firstButton.setFillColor(Theme::Color::ButtonIdle);
  this->_firstButton.setOrigin(this->_firstButton.getSize() / 2.f);

  this->_secondButton.setSize(Theme::Size::Button);
  this->_secondButton.setFillColor(Theme::Color::ButtonIdle);
  this->_secondButton.setOrigin(this->_secondButton.getSize() / 2.f);

  this->_levelText.setCharacterSize(Theme::FontSize::Button);
  this->_levelText.setFillColor(sf::Color::Black);
  this->_levelText.setStyle(sf::Text::Bold);
  this->_levelText.setOrigin(_levelText.getLocalBounds().getCenter());

  this->_easyButton.setSize(Theme::Size::Button);
  this->_easyButton.setFillColor(Theme::Color::ButtonIdle);
  this->_easyButton.setOrigin(this->_easyButton.getSize() / 2.f);

  this->_easyText.setCharacterSize(Theme::FontSize::Button);
  this->_easyText.setFillColor(sf::Color::Black);
  this->_easyText.setStyle(sf::Text::Bold);
  this->_easyText.setOrigin(_easyText.getLocalBounds().getCenter());

  this->_mediumButton.setSize(Theme::Size::Button);
  this->_mediumButton.setFillColor(Theme::Color::ButtonIdle);
  this->_mediumButton.setOrigin(this->_mediumButton.getSize() / 2.f);

  this->_mediumText.setCharacterSize(Theme::FontSize::Button);
  this->_mediumText.setFillColor(sf::Color::Black);
  this->_mediumText.setStyle(sf::Text::Bold);
  this->_mediumText.setOrigin(_mediumText.getLocalBounds().getCenter());

  this->_hardButton.setSize(Theme::Size::Button);
  this->_hardButton.setFillColor(Theme::Color::ButtonIdle);
  this->_hardButton.setOrigin(this->_hardButton.getSize() / 2.f);

  this->_hardText.setCharacterSize(Theme::FontSize::Button);
  this->_hardText.setFillColor(sf::Color::Black);
  this->_hardText.setStyle(sf::Text::Bold);
  this->_hardText.setOrigin(_hardText.getLocalBounds().getCenter());

  this->_startButtonText.setCharacterSize(30);
  this->_startButtonText.setFillColor(sf::Color::White);
  sf::FloatRect btnTextBounds = this->_startButtonText.getLocalBounds();
  this->_startButtonText.setOrigin(btnTextBounds.getCenter());

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
            this->_onStartGame(this->_turnOrder, this->_aiLevel);
        }

        if (this->_firstButton.getGlobalBounds().contains(mousePos)) {
          this->_turnOrder = TurnOrder::PlayerFirst;
        } else if (this->_secondButton.getGlobalBounds().contains(mousePos)) {
          this->_turnOrder = TurnOrder::AIFirst;
        }

        if (this->_easyButton.getGlobalBounds().contains(mousePos)) {
          this->_aiLevel = AILevel::Easy;
        } else if (this->_mediumButton.getGlobalBounds().contains(mousePos)) {
          this->_aiLevel = AILevel::Medium;
        } else if (this->_hardButton.getGlobalBounds().contains(mousePos)) {
          this->_aiLevel = AILevel::Hard;
        }
      }
    }
  }
}

void MenuScene::update(float dt) {
  if (this->_turnOrder == TurnOrder::PlayerFirst) {
    this->_firstButton.setFillColor(Theme::Color::ButtonActive);
    this->_secondButton.setFillColor(Theme::Color::ButtonIdle);
  } else {
    this->_firstButton.setFillColor(Theme::Color::ButtonIdle);
    this->_secondButton.setFillColor(Theme::Color::ButtonActive);
  }

  if (this->_aiLevel == AILevel::Easy) {
    this->_easyButton.setFillColor(Theme::Color::ButtonActive);
    this->_mediumButton.setFillColor(Theme::Color::ButtonIdle);
    this->_hardButton.setFillColor(Theme::Color::ButtonIdle);
  } else if (this->_aiLevel == AILevel::Medium) {
    this->_easyButton.setFillColor(Theme::Color::ButtonIdle);
    this->_mediumButton.setFillColor(Theme::Color::ButtonActive);
    this->_hardButton.setFillColor(Theme::Color::ButtonIdle);
  } else if (this->_aiLevel == AILevel::Hard) {
    this->_easyButton.setFillColor(Theme::Color::ButtonIdle);
    this->_mediumButton.setFillColor(Theme::Color::ButtonIdle);
    this->_hardButton.setFillColor(Theme::Color::ButtonActive);
  }
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
  window.draw(this->_mediumButton);
  window.draw(this->_mediumText);
  window.draw(this->_hardButton);
  window.draw(this->_hardText);
}

void MenuScene::onResize(const sf::Vector2u& windowSize) {
  float w = static_cast<float>(windowSize.x);
  float h = static_cast<float>(windowSize.y);

  this->_titleText.setPosition({w / 2.f, h * 0.3f});
  sf::Vector2f titlePos = this->_titleText.getPosition();
  this->_orderText.setPosition({titlePos.x, titlePos.y + 90.f});
  this->_startButton.setPosition({w / 2.f, h * 0.8f});
  this->_firstButton.setPosition(
      {_orderText.getPosition().x * 0.7f, _orderText.getPosition().y + 50.f});
  this->_secondButton.setPosition(
      {_orderText.getPosition().x * 1.3f, _orderText.getPosition().y + 50.f});
  this->_startButtonText.setPosition(this->_startButton.getPosition());
  this->_firstText.setPosition(this->_firstButton.getPosition());
  this->_secondText.setPosition(this->_secondButton.getPosition());
  this->_levelText.setPosition({w / 2.f, _secondButton.getPosition().y + 80});
  this->_easyButton.setPosition({w / 4.f, _levelText.getPosition().y + 50});
  this->_easyText.setPosition(_easyButton.getPosition());
  this->_mediumButton.setPosition({w / 2.f, _levelText.getPosition().y + 50});
  this->_mediumText.setPosition(_mediumButton.getPosition());
  this->_hardButton.setPosition({(w / 4.f) * 3.f, _levelText.getPosition().y + 50});
  this->_hardText.setPosition(_hardButton.getPosition());
}

void MenuScene::setOnStartGame(std::function<void(TurnOrder turnOrder, AILevel& level)> callback) {
  this->_onStartGame = callback;
}
