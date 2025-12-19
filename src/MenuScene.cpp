#include <MenuScene.hpp>

MenuScene::MenuScene(sf::Font& font, const sf::Vector2u& initalSize)
    : _font(font), _titleText(font, "Gomoku"), _startButtonText(font, "START GAME") {
  this->_titleText.setCharacterSize(80);
  this->_titleText.setFillColor(sf::Color::Black);
  this->_titleText.setStyle(sf::Text::Bold);
  sf::FloatRect titleBounds = this->_titleText.getLocalBounds();
  this->_titleText.setOrigin(titleBounds.getCenter());

  this->_startButton.setSize({250.f, 60.f});
  this->_startButton.setFillColor(sf::Color(100, 100, 100));
  this->_startButton.setOrigin(this->_startButton.getSize() / 2.f);

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
            this->_onStartGame();
        }
      }
    }
  }
}

void MenuScene::update(float dt) {
}

void MenuScene::render(sf::RenderWindow& window) {
  window.clear(sf::Color(50, 50, 50));
  window.draw(this->_titleText);
  window.draw(this->_startButton);
  window.draw(this->_startButtonText);
}

void MenuScene::onResize(const sf::Vector2u& windowSize) {
  float w = static_cast<float>(windowSize.x);
  float h = static_cast<float>(windowSize.y);

  this->_titleText.setPosition({w / 2.f, h * 0.3f});
  this->_startButton.setPosition({w / 2.f, h * 0.6f});
  this->_startButtonText.setPosition(this->_startButton.getPosition());
}

void MenuScene::setOnStartGame(std::function<void()> callback) {
  this->_onStartGame = callback;
}
