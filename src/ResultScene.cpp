#include <ResultScene.hpp>

ResultScene::ResultScene(sf::Font& font, const sf::Vector2u& initalSize)
    : _font(font),
      _titleText(font, "Result"),
      _winnerText(font, ""),
      _backButtonText(font, "Back to Menu") {
  this->_titleText.setCharacterSize(Theme::FontSize::Title);
  this->_titleText.setFillColor(sf::Color::White);
  this->_titleText.setStyle(sf::Text::Bold);
  sf::FloatRect titleBounds = this->_titleText.getLocalBounds();
  this->_titleText.setOrigin(titleBounds.getCenter());

  this->_winnerText.setCharacterSize(Theme::FontSize::Header);
  this->_winnerText.setFillColor(sf::Color::White);
  this->_winnerText.setStyle(sf::Text::Bold);
  this->_winnerText.setOrigin(this->_winnerText.getLocalBounds().getCenter());

  this->_backButtonText.setCharacterSize(Theme::FontSize::Button);
  this->_backButtonText.setFillColor(sf::Color::Black);
  this->_backButtonText.setStyle(sf::Text::Bold);
  this->_backButtonText.setOrigin(this->_backButtonText.getLocalBounds().getCenter());

  this->_backButton.setSize(Theme::Size::Button);
  this->_backButton.setFillColor(Theme::Color::ButtonActive);
  this->_backButton.setOrigin(this->_backButton.getSize() / 2.f);

  onResize(initalSize);
}

void ResultScene::handleEvents(const EventList& events) {
  for (const auto e : events) {
    if (const auto* mousePtr = e->getIf<sf::Event::MouseButtonPressed>()) {
      if (mousePtr->button == sf::Mouse::Button::Left) {
        sf::Vector2f mousePos(static_cast<float>(mousePtr->position.x),
                              static_cast<float>(mousePtr->position.y));
        if (this->_backButton.getGlobalBounds().contains(mousePos)) {
          if (this->_onBack) {
            _onBack();
          }
        }
      }
    }
  }
}

void ResultScene::update(float dt) {
}

void ResultScene::render(sf::RenderWindow& window) {
  window.clear(Theme::Color::BoardDarkened);

  if (this->_backgroundSprite) {
    window.draw(*_backgroundSprite);
  }
  window.draw(this->_titleText);
  window.draw(this->_winnerText);
  window.draw(this->_backButton);
  window.draw(this->_backButtonText);
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
  this->_backButton.setPosition({w / 2.f, h * 0.95f});
  this->_backButtonText.setPosition(this->_backButton.getPosition());
}

void ResultScene::setBackground(const sf::Window& window) {
  if (this->_backgroundTexture.resize(window.getSize())) {
    this->_backgroundTexture.update(window);

    this->_backgroundSprite = std::make_unique<sf::Sprite>(this->_backgroundTexture);

    this->_backgroundSprite->setColor(Theme::Color::Background);
  }
}

void ResultScene::setWinner(const std::string& winner) {
  this->_winnerText.setString(winner);
  this->_winnerText.setOrigin(this->_winnerText.getLocalBounds().getCenter());
}

void ResultScene::setOnBack(std::function<void()> func) {
  this->_onBack = func;
}
