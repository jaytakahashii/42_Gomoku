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
  sf::Font& _font;
  sf::Text _titleText;
  sf::RectangleShape _startButton;
  sf::Text _startButtonText;
  std::function<void()> _onStartGame;
};

#endif
