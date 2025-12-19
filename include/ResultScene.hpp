#ifndef RESULTSCENE_HPP
#define RESULTSCENE_HPP

#include <Scene.hpp>

class ResultScene : public Scene {
 public:
  ResultScene(sf::Font& font, const sf::Vector2u& initialSize);
  void handleEvents(const EventList& events);
  void update(float dt);
  void render(sf::RenderWindow& window);
  void onResize(const sf::Vector2u& windowSize);

 private:
  sf::Font& _font;
  sf::Text _titleText;
};

#endif
