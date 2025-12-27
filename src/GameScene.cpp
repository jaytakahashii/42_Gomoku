#include <GameScene.hpp>

GameScene::GameScene(sf::Font& font, const sf::Vector2u& initalSize) : _font(font) {
  onResize(initalSize);
}

void GameScene::handleEvents(const EventList& events) {
  for (const auto e : events) {
    if (const auto* mousePtr = e->getIf<sf::Event::MouseButtonPressed>()) {
      if (mousePtr->button == sf::Mouse::Button::Left) {
        sf::Vector2f mousePos(static_cast<float>(mousePtr->position.x),
                              static_cast<float>(mousePtr->position.y));
        handleClick(mousePos.x, mousePos.y);
      }
    }
  }
}

void GameScene::displayTimedMessage(const std::string& str, sf::Vector2f pos) {
  this->_activeMessages.emplace_back(this->_font, str, pos);
}

void GameScene::handleClick(int x, int y) {
  int col = static_cast<int>(std::round((x - _boardOffset.x) / _cellSize));
  int row = static_cast<int>(std::round((y - _boardOffset.y) / _cellSize));

  if (col >= 0 && col < static_cast<int>(_boardSize) && row >= 0 &&
      row < static_cast<int>(_boardSize)) {
    // makeMoveが成功（ルール上OK）なら、内部状態が更新される
    if (_board.makeMove(col, row)) {
      float posX = _boardOffset.x + static_cast<float>(col * _cellSize);
      float posY = _boardOffset.y + static_cast<float>(row * _cellSize);
      // static bool fg = false;
      // fg = !fg;
      // displayTimedMessage(fg ? "Hello" : "World", {posX, posY});

      // TODO: SEを鳴らすなどの処理があればここに書く

      if (_board.checkWin()) {
        // TODO: debug用
        printf("Player %s wins!\n", _board.getCurrentTurn() == Player::BLACK ? "Black" : "White");
        std::string winner = _board.getCurrentTurn() == Player::BLACK ? "Black" : "White";
        if (_onGameOver)
          _onGameOver(winner);
      }

      _board.changeTurn();
    }
  }
}

void GameScene::render(sf::RenderWindow& window) {
  window.clear(Theme::Color::Board);

  const float boardLength = static_cast<float>((_boardSize - 1) * _cellSize);

  for (unsigned int i = 0; i < _boardSize; ++i) {
    float iPos = static_cast<float>(i * _cellSize);

    sf::Vertex h_line[] = {
        sf::Vertex{{_boardOffset.x, _boardOffset.y + iPos}, sf::Color::Black},
        sf::Vertex{{_boardOffset.x + boardLength, _boardOffset.y + iPos}, sf::Color::Black}};
    window.draw(h_line, 2, sf::PrimitiveType::Lines);

    sf::Vertex v_line[] = {
        sf::Vertex{{_boardOffset.x + iPos, _boardOffset.y}, sf::Color::Black},
        sf::Vertex{{_boardOffset.x + iPos, _boardOffset.y + boardLength}, sf::Color::Black}};
    window.draw(v_line, 2, sf::PrimitiveType::Lines);
  }

  sf::CircleShape stone(15.f);
  stone.setOrigin(sf::Vector2f(15.0f, 15.f));

  for (unsigned int y = 0; y < _boardSize; ++y) {
    for (unsigned int x = 0; x < _boardSize; ++x) {
      Player p = _board.getStoneAt(x, y);

      if (p != Player::NONE) {
        float posX = _boardOffset.x + static_cast<float>(x * _cellSize);
        float posY = _boardOffset.y + static_cast<float>(y * _cellSize);
        stone.setPosition(sf::Vector2f(posX, posY));

        stone.setFillColor(p == Player::BLACK ? sf::Color::Black : sf::Color::White);
        window.draw(stone);
      }
    }
  }
  for (const auto& message : this->_activeMessages) {
    window.draw(message.text);
  }
}

void GameScene::update(float df) {
  auto it = this->_activeMessages.begin();
  while (it != this->_activeMessages.end()) {
    it->timer -= df;

    if (it->timer <= 0.0f) {
      it = this->_activeMessages.erase(it);
    } else {
      float alphaProgress = std::min(1.0f, it->timer / 0.5f);
      std::uint8_t alpha = static_cast<std::uint8_t>(255 * alphaProgress);

      sf::Color color = it->text.getFillColor();
      color.a = alpha;
      it->text.setFillColor(color);

      sf::Color outColor = it->text.getOutlineColor();
      outColor.a = alpha;
      it->text.setOutlineColor(outColor);

      it->text.move({0.f, -40.f * df});
      ++it;
    }
  }
}

void GameScene::onResize(const sf::Vector2u& windowSize) {
  // render(windowSize);
  float w = static_cast<float>(windowSize.x);
  float h = static_cast<float>(windowSize.y);

  // 盤面全体のピクセル幅・高さ（19本の線 = 18マス分）
  float boardPixelSize = static_cast<float>((_boardSize - 1) * _cellSize);

  // 画面中央になるようにオフセットを計算
  // (画面幅 - 盤面幅) / 2 = 左側の余白
  _boardOffset.x = (w - boardPixelSize) / 2.f;
  _boardOffset.y = (h - boardPixelSize) / 2.f;
}

void GameScene::setOnGameOver(std::function<void(const std::string& winner)> callback) {
  this->_onGameOver = callback;
}

void GameScene::setAILevel(AILevel& level) {
  this->_aiLevel = level;
}

void GameScene::setTurnOrder(TurnOrder turnOrder) {
  this->_turnOrder = turnOrder;
}

GameScene::FloatingMessage::FloatingMessage(const sf::Font& font, const std::string& str,
                                            sf::Vector2f pos)
    : text(font), timer(1.0f) {
  text.setString(str);
  text.setCharacterSize(Theme::FontSize::Header);
  text.setFillColor(Theme::Color::AlertText);
  text.setOutlineColor(sf::Color::Black);
  text.setOutlineThickness(1.5f);
  sf::FloatRect textRect = text.getLocalBounds();
  text.setOrigin(
      {textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f});

  text.setPosition({pos.x, pos.y - 20.f});

  sf::Color color = text.getFillColor();
  color.a = 255;
  text.setFillColor(color);
}
