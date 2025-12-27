#include <Gomoku.hpp>

Gomoku::Gomoku() : _window(sf::VideoMode({1080, 1000}), "Gomoku"), font() {
  initFont("arial.ttf");
  this->_window.setMinimumSize(sf::Vector2u(1080, 1000));

  this->_menuScene = std::make_unique<MenuScene>(font, this->_window.getSize());
  this->_gameScene = std::make_unique<GameScene>(font, this->_window.getSize());
  this->_resultScene = std::make_unique<ResultScene>(font, this->_window.getSize());
  this->_currentScene = this->_menuScene.get();

  this->_menuScene->setOnStartGame([this](TurnOrder turnOrder, AILevel& level) {
    this->_gameScene.get()->setAILevel(level);
    this->_gameScene.get()->setTurnOrder(turnOrder);
    changeScene(this->_gameScene.get());
  });
  this->_gameScene->setOnGameOver([this](const std::string& winner) { initOnGameOver(winner); });
  this->_resultScene->setOnBack([this] {
    changeScene(this->_menuScene.get());
    this->_gameScene = std::make_unique<GameScene>(font, this->_window.getSize());
    this->_gameScene->setOnGameOver([this](const std::string& winner) { initOnGameOver(winner); });
  });
}
void Gomoku::initOnGameOver(const std::string& winner) {
  this->_window.clear();
  this->_gameScene->render(this->_window);
  this->_resultScene.get()->setBackground(this->_window);
  this->_resultScene.get()->setWinner(winner);
  changeScene(this->_resultScene.get());
}

void Gomoku::run() {
  sf::Clock clock;
  while (this->_window.isOpen()) {
    sf::Time dt = clock.restart();
    float deltaTime = dt.asSeconds();
    EventList events = getEventList();
    this->_currentScene->handleEvents(events);
    this->_currentScene->update(deltaTime);
    this->_window.clear();
    this->_currentScene->render(this->_window);
    this->_window.display();
  }
}

void Gomoku::initFont(const std::string font) {
  if (!this->font.openFromFile(font)) {
    std::cerr << "Failed to load font!" << std::endl;
    return;
  }
}

Gomoku::EventList Gomoku::getEventList() {
  EventList events;
  while (const std::optional e = _window.pollEvent()) {
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

void Gomoku::changeScene(Scene* nextScene) {
  if (nextScene) {
    nextScene->onResize(this->_window.getSize());
    this->_currentScene = nextScene;
  }
}
