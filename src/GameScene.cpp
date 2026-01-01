#include <GameScene.hpp>

GameScene::GameScene(sf::Font& font, const sf::Vector2u& initalSize)
    : _font(font),
      _countWhiteCaptures(font, "White Captured: 0"),
      _countBlackCaptures(font, "Black Captured: 0"),
      _turnNotification(font, "Your Turn"),
      _aiInfoText(font, "AI Time: 0.00s"),
      _undoText(font, "Undo"),
      _aiAssistText(font, "AI Assist") {
  this->_countWhiteCaptures.setCharacterSize(Theme::FontSize::Text);
  this->_countWhiteCaptures.setFillColor(Theme::Color::Text);
  this->_countWhiteCaptures.setOrigin(this->_countWhiteCaptures.getGlobalBounds().getCenter());

  this->_countBlackCaptures.setCharacterSize(Theme::FontSize::Text);
  this->_countBlackCaptures.setFillColor(Theme::Color::Text);
  this->_countBlackCaptures.setOrigin(this->_countBlackCaptures.getGlobalBounds().getCenter());

  this->_turnNotification.setCharacterSize(Theme::FontSize::Header);
  this->_turnNotification.setFillColor(Theme::Color::AlertText);
  this->_turnNotification.setOrigin(this->_turnNotification.getGlobalBounds().getCenter());

  this->_aiInfoText.setCharacterSize(Theme::FontSize::Text);
  this->_aiInfoText.setFillColor(Theme::Color::Text);

  this->_undoText.setCharacterSize(Theme::FontSize::Button);
  this->_undoText.setFillColor(sf::Color::Black);
  this->_undoText.setOrigin(_undoText.getLocalBounds().getCenter());

  this->_undoButton.setSize(Theme::Size::Button);
  this->_undoButton.setFillColor(Theme::Color::ButtonActive);
  this->_undoButton.setOrigin(this->_undoButton.getSize() / 2.f);

  this->_aiAssistText.setCharacterSize(Theme::FontSize::Button);
  this->_aiAssistText.setFillColor(sf::Color::Black);
  this->_aiAssistText.setOrigin(_aiAssistText.getLocalBounds().getCenter());

  this->_aiAssistButton.setSize(Theme::Size::Button);
  this->_aiAssistButton.setFillColor(Theme::Color::ButtonActive);
  this->_aiAssistButton.setOrigin(this->_aiAssistButton.getSize() / 2.f);

  onResize(initalSize);
}

void GameScene::handleEvents(const EventList& events) {
  if (this->_board.getCurrentPlayer() == Player::AI)
    return;
  for (const auto e : events) {
    if (const auto* mousePtr = e->getIf<sf::Event::MouseButtonPressed>()) {
      if (mousePtr->button == sf::Mouse::Button::Left) {
        sf::Vector2f mousePos(static_cast<float>(mousePtr->position.x),
                              static_cast<float>(mousePtr->position.y));
        if (this->_undoButton.getGlobalBounds().contains(mousePos)) {
          _onUndo();
        } else if (this->_aiAssistButton.getGlobalBounds().contains(mousePos)) {
          _onAIAssist();
        } else
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
    float posX = _boardOffset.x + static_cast<float>(col * _cellSize);
    float posY = _boardOffset.y + static_cast<float>(row * _cellSize);
    if (_board.makeMove(col, row)) {
      this->_hintMove = {-1, -1};

      if (_board.getCapturedStatus()) {
        displayTimedMessage("Capture", {posX, posY});
      }

      _updateCaptures();

      if (_board.checkWin()) {
        if (_onGameOver)
          _onGameOver("You");
      }
      _board.changeTurn();
    }
    if (_board.getDoubleThreeStatus()) {
      displayTimedMessage("DoubleThree", {posX, posY});
      _board.setDoubleThreeStatus(false);  // リセット
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
      Color p = _board.getColorAt(x, y);

      if (p != Color::NONE) {
        float posX = _boardOffset.x + static_cast<float>(x * _cellSize);
        float posY = _boardOffset.y + static_cast<float>(y * _cellSize);
        stone.setPosition(sf::Vector2f(posX, posY));

        stone.setFillColor(p == Color::BLACK ? sf::Color::Black : sf::Color::White);
        window.draw(stone);
      }
    }
  }
  for (const auto& message : this->_activeMessages) {
    window.draw(message.text);
  }
  window.draw(this->_countWhiteCaptures);
  window.draw(this->_countBlackCaptures);

  if (this->_board.getCurrentPlayer() == Player::HUMAN) {
    window.draw(this->_turnNotification);
  }

  if (this->_hintMove.x != -1 && this->_hintMove.y != -1)
    window.draw(_makeHintCircle());

  window.draw(this->_aiInfoText);

  window.draw(this->_undoButton);
  window.draw(this->_undoText);

  window.draw(this->_aiAssistButton);
  window.draw(this->_aiAssistText);
}

void GameScene::update(float df) {
  _handleMessage(df);

  _notifyPlayerTurn(df);

  _handleHint();

  _handleAIProcess(df);
}

void GameScene::_applyAIMove(Move move) {
  this->_board.makeMove(move.x, move.y);

  float posX = _boardOffset.x + static_cast<float>(move.x * _cellSize);
  float posY = _boardOffset.y + static_cast<float>(move.y * _cellSize);

  if (this->_board.getCapturedStatus()) {
    displayTimedMessage("Capture", {posX, posY});
  }

  int whiteCaptures = this->_board.getWhiteCaptures();
  int blackCaptures = this->_board.getBlackCaptures();
  this->_countWhiteCaptures.setString("White Captured: " + std::to_string(whiteCaptures));
  this->_countBlackCaptures.setString("Black Captured: " + std::to_string(blackCaptures));

  if (this->_board.checkWin()) {
    if (this->_onGameOver)
      this->_onGameOver("AI");
    return;
  }

  _board.changeTurn();
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

  this->_countWhiteCaptures.setPosition({w / 4.f, h / 9.5f});
  this->_countBlackCaptures.setPosition({w * 3 / 4.f, h / 9.5f});

  this->_turnNotification.setPosition({w / 2.f, 50.f});

  this->_aiInfoText.setPosition({20.f, h - 50.f});

  this->_undoButton.setPosition({w * 3 / 4, h - 50.f});
  this->_undoText.setPosition(_undoButton.getPosition());

  this->_aiAssistButton.setPosition({w * 2 / 4, h - 50.f});
  this->_aiAssistText.setPosition(_aiAssistButton.getPosition());
}

void GameScene::setOnGameOver(std::function<void(const std::string& winner)> callback) {
  this->_onGameOver = callback;
}

void GameScene::setAILevel(AILevel& level) {
  this->_aiLevel = level;
}

void GameScene::setTurnOrder(TurnOrder turnOrder) {
  this->_board.setupPlayers(turnOrder);
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

void GameScene::_onUndo() {
  if (_isAIThinking || _isCalculatingHint)
    return;

  if (this->_board.undo()) {
    this->_board.undo();
  }
  _hintMove = {-1, -1};
  _updateCaptures();
}

void GameScene::_updateCaptures() {
  int whiteCaptures = this->_board.getWhiteCaptures();
  int blackCaptures = this->_board.getBlackCaptures();
  this->_countWhiteCaptures.setString("White Captured: " + std::to_string(whiteCaptures));
  this->_countBlackCaptures.setString("Black Captured: " + std::to_string(blackCaptures));
}

void GameScene::_onAIAssist() {
  if (this->_isAIThinking || this->_isCalculatingHint ||
      this->_board.getCurrentPlayer() != Player::HUMAN) {
    return;
  }

  this->_isCalculatingHint = true;
  this->_hintMove = {-1, -1};

  AI ai;
  Board boardCopy = this->_board;
  Color turnColor = this->_board.getCurrentTurn();
  AILevel level = AILevel::Hard;

  this->_hintFuture = std::async(std::launch::async, [ai, boardCopy, turnColor, level]() mutable {
    return ai.getBestMove(boardCopy, turnColor, level);
  });
}

void GameScene::_handleMessage(float df) {
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

void GameScene::_notifyPlayerTurn(float df) {
  if (this->_board.getCurrentPlayer() == Player::HUMAN) {
    this->_turnAnimTimer += df;

    float sinVal = std::sin(this->_turnAnimTimer * 5.0f);
    std::uint8_t alpha = static_cast<std::uint8_t>(190 + 65 * sinVal);

    sf::Color color = this->_turnNotification.getFillColor();
    color.a = alpha;
    this->_turnNotification.setFillColor(color);
  }
}

void GameScene::_handleHint() {
  if (this->_isCalculatingHint) {
    if (this->_hintFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      Move bestMove = this->_hintFuture.get();
      this->_hintMove = {bestMove.x, bestMove.y};
      this->_isCalculatingHint = false;
    }
  }
}

void GameScene::_handleAIProcess(float df) {
  if (this->_board.getCurrentPlayer() == Player::AI) {
    if (!this->_isAIThinking) {
      this->_aiMoveTimer += df;

      if (this->_aiMoveTimer >= 0.5f) {
        this->_isAIThinking = true;
        this->_aiMoveTimer = 0.0f;

        AI ai;
        Color turnColor = this->_board.getCurrentTurn();
        Board boardCopy = this->_board;
        AILevel level = this->_aiLevel;

        this->_aiClock.restart();
        this->_aiFuture =
            std::async(std::launch::async, [ai, boardCopy, turnColor, level]() mutable {
              return ai.getBestMove(boardCopy, turnColor, level);
            });
      }
    } else {
      float elapsed = this->_aiClock.getElapsedTime().asSeconds();

      std::stringstream ss;
      ss << "AI Thinking... " << std::fixed << std::setprecision(2) << elapsed << "s";
      this->_aiInfoText.setString(ss.str());
      if (this->_aiFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        Move bestMove = this->_aiFuture.get();
        float finalTime = this->_aiClock.getElapsedTime().asSeconds();
        std::stringstream ssFinal;
        ssFinal << "AI Time: " << std::fixed << std::setprecision(2) << finalTime << "s";
        this->_aiInfoText.setString(ssFinal.str());
        _applyAIMove(bestMove);
        this->_isAIThinking = false;
      }
    }
  } else {
    this->_aiMoveTimer = 0.0f;
    this->_isAIThinking = false;
  }
}

sf::CircleShape GameScene::_makeHintCircle() const {
  float posX = this->_boardOffset.x + static_cast<float>(this->_hintMove.x * this->_cellSize);
  float posY = this->_boardOffset.y + static_cast<float>(this->_hintMove.y * this->_cellSize);

  sf::CircleShape hintShape(10.f);
  hintShape.setOrigin({10.f, 10.f});
  hintShape.setPosition({posX, posY});

  float sinVal = std::sin(this->_turnAnimTimer * 8.0f);
  std::uint8_t alpha = static_cast<std::uint8_t>(150 + 100 * sinVal);

  hintShape.setFillColor(sf::Color(0, 255, 0, alpha));
  hintShape.setOutlineColor(sf::Color(255, 255, 255, alpha));
  hintShape.setOutlineThickness(2.f);

  return hintShape;
}
