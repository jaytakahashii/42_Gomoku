#ifndef GOMOKU_HPP
#define GOMOKU_HPP

#include <GameScene.hpp>
#include <MenuScene.hpp>
#include <ResultScene.hpp>
#include <SFML/Graphics.hpp>
#include <Scene.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

class Gomoku {
 public:
  sf::Font font;

  Gomoku();

  void run();

 private:
  sf::RenderWindow _window;
  std::unique_ptr<MenuScene> _menuScene;
  std::unique_ptr<GameScene> _gameScene;
  std::unique_ptr<ResultScene> _resultScene;
  Scene* _currentScene = nullptr;

  using EventList = std::vector<std::optional<sf::Event>>;
  void initFont(const std::string font);
  EventList getEventList();
};

#endif
