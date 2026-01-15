#pragma once

#include "SFML/Graphics.hpp"

class Tooltip {
 public:
  Tooltip(sf::Font& font);

  void show(const std::string& str, sf::Vector2f mousePos, const sf::RenderWindow& window);

  void hide();

  void draw(sf::RenderWindow& window);

 private:
  sf::RectangleShape _background;
  sf::Text _text;
  bool _visible = false;
};
