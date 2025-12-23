#include <ResultScene.hpp>

ResultScene::ResultScene(sf::Font& font, const sf::Vector2u& initalSize)
    : _font(font), _titleText(font, "Result"), _winnerText(font, "") {
  this->_titleText.setCharacterSize(Theme::FontSize::Title);
  this->_titleText.setFillColor(Theme::Color::Text);
  this->_titleText.setStyle(sf::Text::Bold);
  sf::FloatRect titleBounds = this->_titleText.getLocalBounds();
  this->_titleText.setOrigin(titleBounds.getCenter());

  this->_winnerText.setCharacterSize(Theme::FontSize::Header);
  this->_winnerText.setFillColor(sf::Color::Black);
  this->_winnerText.setStyle(sf::Text::Bold);
  this->_winnerText.setOrigin(_winnerText.getLocalBounds().getCenter());

  onResize(initalSize);
}

void ResultScene::handleEvents(const EventList& events) {
  for (const auto e : events) {
  }
}

void ResultScene::update(float dt) {
}

void ResultScene::render(sf::RenderWindow& window) {
  window.clear(Theme::Color::BoardDarkened);

  if (_backgroundSprite) {
    window.draw(*_backgroundSprite);
  }
  window.draw(_titleText);
  window.draw(_winnerText);
}

void ResultScene::onResize(const sf::Vector2u& windowSize) {
  float w = static_cast<float>(windowSize.x);
  float h = static_cast<float>(windowSize.y);

  this->_titleText.setPosition({w / 2.f, h * 0.3f});
  this->_winnerText.setPosition({w / 2.f, h * 0.4f});
  if (this->_backgroundSprite) {
    this->_backgroundSprite->setOrigin({0.f, 0.f});
    this->_backgroundSprite->setPosition({0.f, 0.f});

    sf::FloatRect textureRect = this->_backgroundSprite->getLocalBounds();
    if (textureRect.size.x > 0 && textureRect.size.y > 0) {
      float scaleX = w / textureRect.size.x;
      float scaleY = h / textureRect.size.y;

      float scale = std::min(scaleX, scaleY);
      this->_backgroundSprite->setScale({scale, scale});

      float offsetX = (w - (textureRect.size.x * scale)) / 2.f;
      float offsetY = (h - (textureRect.size.y * scale)) / 2.f;
      this->_backgroundSprite->setPosition({offsetX, offsetY});
    }
  }
}

void ResultScene::setBackground(const sf::Window& window) {
  if (this->_backgroundTexture.resize(window.getSize())) {
    _backgroundTexture.update(window);

    _backgroundSprite = std::make_unique<sf::Sprite>(_backgroundTexture);

    _backgroundSprite->setColor(DarkGrey);
  }
}

void ResultScene::setWinner(const std::string& winner) {
  this->_winnerText.setString("Player " + winner + " wins!");
  this->_winnerText.setOrigin(_winnerText.getLocalBounds().getCenter());
}
