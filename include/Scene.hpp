#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

class Scene {
 public:
  virtual ~Scene() = default;

  using EventList = std::vector<std::optional<sf::Event>>;

  virtual void handleEvents(const EventList& events) = 0;
  virtual void update(float dt) = 0;
  virtual void render(sf::RenderWindow& window) = 0;
  virtual void onResize(const sf::Vector2u& windowSize) = 0;
};
