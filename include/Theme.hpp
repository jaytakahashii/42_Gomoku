#ifndef THEME_HPP
#define THEME_HPP

#include <SFML/Graphics.hpp>

namespace Theme {

namespace Color {
const sf::Color Text = sf::Color::Black;
const sf::Color Background = sf::Color(100, 100, 100);
const sf::Color ButtonIdle = sf::Color(80, 80, 80);
const sf::Color ButtonActive = sf::Color(180, 180, 180);
const sf::Color Board = sf::Color(200, 160, 100);
const sf::Color BoardDarkened = sf::Color(200 * 100 / 255, 160 * 100 / 255, 100 * 100 / 255);
const sf::Color AlertText = sf::Color(180, 50, 50);
}  // namespace Color

namespace FontSize {
const unsigned int Title = 80;
const unsigned int Header = 45;
const unsigned int Text = 35;
const unsigned int Button = 25;
}  // namespace FontSize

namespace Size {
const sf::Vector2f Button = {250.f, 60.f};
}
}  // namespace Theme

#endif
