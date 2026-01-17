#pragma once
#include <Scene.hpp>
#include <Theme.hpp>

class ResultScene : public Scene {
 public:
  ResultScene(Fonts& fonts, const sf::Vector2u& initialSize);
  void handleEvents(const EventList& events);
  void update(float dt);
  void render(sf::RenderWindow& window);
  void onResize(const sf::Vector2u& windowSize);

  void setBackground(const sf::Window& window);
  void setWinner(const std::string& winner);
  void setOnBack(std::function<void()> func);

 private:
  Fonts& _fonts;
  sf::Text _titleText;
  sf::Texture _backgroundTexture;
  std::unique_ptr<sf::Sprite> _backgroundSprite;
  sf::Text _winnerText;
  sf::RectangleShape _backButton;
  sf::Text _backButtonText;
  std::function<void()> _onBack;
};
