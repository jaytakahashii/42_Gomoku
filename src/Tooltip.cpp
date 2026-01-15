#include "Tooltip.hpp"

#include "Theme.hpp"

Tooltip::Tooltip(sf::Font& font) : _text(font) {
  this->_text.setCharacterSize(Theme::FontSize::Tooltip);
  this->_text.setFillColor(Theme::Color::Text);

  this->_background.setFillColor(Theme::Color::ButtonIdle);
  this->_background.setOutlineColor(sf::Color::White);
  this->_background.setOutlineThickness(1.f);
}

void Tooltip::hide() {
  this->_visible = false;
}

void Tooltip::show(const std::string& str, sf::Vector2f mousePos, const sf::RenderWindow& window) {
  this->_text.setString(str);
  this->_visible = true;

  sf::FloatRect textBounds = this->_text.getLocalBounds();
  sf::Vector2f bgSize = {textBounds.size.x + 20.f, textBounds.size.y + 25.f};
  this->_background.setSize(bgSize);

  sf::Vector2f viewSize = window.getView().getSize();
  sf::Vector2f viewCenter = window.getView().getCenter();
  sf::Vector2f viewTopLeft = viewCenter - (viewSize / 2.f);

  float offset = 15.f;
  float finalX = mousePos.x + offset;
  float finalY = mousePos.y + offset;

  if (finalX + bgSize.x > viewTopLeft.x + viewSize.x) {
    finalX = mousePos.x - offset - bgSize.x;
  }

  if (finalY + bgSize.y > viewTopLeft.y + viewSize.y) {
    finalY = mousePos.y - offset - bgSize.y;
  }

  this->_background.setPosition({finalX, finalY});
  this->_text.setPosition({finalX + 10.f, finalY + 10.f});
}

void Tooltip::draw(sf::RenderWindow& window) {
  if (this->_visible) {
    window.draw(this->_background);
    window.draw(this->_text);
  }
}
