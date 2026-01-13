#include <Gomoku.hpp>

Gomoku::Gomoku() : _window(sf::VideoMode({1080, 1000}), "Gomoku"), font() {
  _initFont("arial.ttf");
  this->_isRunning = true;
  this->_window.setMinimumSize(sf::Vector2u(1080, 1000));

  this->_menuScene = std::make_unique<MenuScene>(font, this->_window.getSize());
  this->_gameScene = std::make_unique<GameScene>(font, this->_window.getSize());
  this->_resultScene = std::make_unique<ResultScene>(font, this->_window.getSize());
  this->_currentScene = this->_menuScene.get();

  this->_menuScene->setOnStartGame([this](TurnOrder& turnOrder, AILevel& level, OpeningRule& rule) {
    this->_gameScene.get()->setAILevel(level);
    this->_gameScene.get()->setTurnOrder(turnOrder);
    this->_gameScene.get()->setOpeningRule(rule);
    _changeScene(this->_gameScene.get());
  });
  this->_gameScene->setOnGameOver([this](const std::string& winner) { _initOnGameOver(winner); });
  this->_gameScene->setOnEsc([this]() {
    _changeScene(this->_menuScene.get());
    this->_gameScene->reset();
  });
  this->_resultScene->setOnBack([this] {
    _changeScene(this->_menuScene.get());
    this->_gameScene->reset();
  });
}
void Gomoku::_initOnGameOver(const std::string& winner) {
  this->_window.clear();
  this->_gameScene->render(this->_window);
  this->_resultScene.get()->setBackground(this->_window);
  this->_resultScene.get()->setWinner(winner);
  _changeScene(this->_resultScene.get());
}

void Gomoku::run() {
  sf::Clock clock;
  while (this->_window.isOpen() && this->_isRunning) {
    sf::Time dt = clock.restart();
    float deltaTime = dt.asSeconds();
    EventList events = _getEventList();
    this->_currentScene->handleEvents(events);
    this->_currentScene->update(deltaTime);
    this->_window.clear();
    this->_currentScene->render(this->_window);
    this->_window.display();
  }
}

void Gomoku::stop() {
  this->_isRunning = false;
}

void Gomoku::_initFont(const std::string font) {
  if (!this->font.openFromFile(font)) {
    std::cerr << "Failed to load font!" << std::endl;
    return;
  }
}

Gomoku::EventList Gomoku::_getEventList() {
  EventList events;
  while (const std::optional e = this->_window.pollEvent()) {
    if (e->is<sf::Event::Closed>()) {
      this->_window.close();
    } else if (const auto* resized = e->getIf<sf::Event::Resized>()) {
      sf::FloatRect visibleArea(
          {0.f, 0.f}, {static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)});
      this->_window.setView(sf::View(visibleArea));
      this->_currentScene->onResize(resized->size);
    } else {
      events.push_back(e);
    }
  }
  return events;
}

void Gomoku::_changeScene(Scene* nextScene) {
  if (nextScene) {
    nextScene->onResize(this->_window.getSize());
    this->_currentScene = nextScene;
  }
}
