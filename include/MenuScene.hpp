#ifndef MENUSCENE_HPP
#define MENUSCENE_HPP

#include <Scene.hpp>
#include <functional>

class MenuScene : public Scene {
 public:
  MenuScene(sf::Font& font, const sf::Vector2u& initialSize);

  void handleEvents(const EventList& events);
  void update(float dt);
  void render(sf::RenderWindow& window);
  void onResize(const sf::Vector2u& windowSize);

  void setOnStartGame(std::function<void()> callback);

 private:
  enum Level { Easy, Medium, Hard };
  sf::Font& _font;
  sf::Text _titleText;
  sf::RectangleShape _startButton;
  sf::Text _orderText;
  sf::RectangleShape _firstButton;
  sf::RectangleShape _secondButton;
  sf::RectangleShape _easyButton;
  sf::RectangleShape _mediumButton;
  sf::RectangleShape _hardButton;
  sf::Text _levelText;
  sf::Text _mediumText;
  sf::Text _easyText;
  sf::Text _hardText;
  sf::Text _firstText;
  sf::Text _secondText;
  sf::Text _startButtonText;
  bool _isFirst = true;
  Level _level = Level::Medium;
  std::function<void()> _onStartGame;
};

#endif
