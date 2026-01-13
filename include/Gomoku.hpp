#pragma once

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

#include "Zobrist.hpp"

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
  void _initFont(const std::string font);
  EventList _getEventList();
  void _changeScene(Scene* nextScene);
  void _initOnGameOver(const std::string& winner);
};
