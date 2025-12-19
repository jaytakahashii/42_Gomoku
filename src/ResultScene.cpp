
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
  window.clear(sf::Color(50, 50, 50));
  window.draw(this->_titleText);
}

void ResultScene::onResize(const sf::Vector2u& windowSize) {
  float w = static_cast<float>(windowSize.x);
  float h = static_cast<float>(windowSize.y);

  this->_titleText.setPosition({w / 2.f, h * 0.3f});
}
