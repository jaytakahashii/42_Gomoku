#include <ResultScene.hpp>

ResultScene::ResultScene(sf::Font& font, const sf::Vector2u& initalSize)
    : _font(font), _titleText(font, "Result") {
  this->_titleText.setCharacterSize(80);
  this->_titleText.setFillColor(sf::Color::Black);
  this->_titleText.setStyle(sf::Text::Bold);
  sf::FloatRect titleBounds = this->_titleText.getLocalBounds();
  this->_titleText.setOrigin(titleBounds.getCenter());

  onResize(initalSize);
}

void ResultScene::handleEvents(const EventList& events) {
  for (const auto e : events) {
  }
}

void ResultScene::update(float dt) {
}

void ResultScene::render(sf::RenderWindow& window) {
  if (_backgroundSprite) {
    window.draw(*_backgroundSprite);
  }
  window.draw(_titleText);
}

void ResultScene::onResize(const sf::Vector2u& windowSize) {
  float w = static_cast<float>(windowSize.x);
  float h = static_cast<float>(windowSize.y);

  this->_titleText.setPosition({w / 2.f, h * 0.3f});
}

void ResultScene::setBackground(const sf::Window& window) {
  _backgroundTexture.update(window);

  _backgroundSprite = std::make_unique<sf::Sprite>(_backgroundTexture);

  _backgroundSprite->setColor(sf::Color(100, 100, 100));
}
