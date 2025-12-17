#include <SFML/Graphics.hpp>

int main() {
    // SFML 3.0: VideoModeはサイズ(Vector2u)を渡す方式に変更
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Gomoku");

    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);
    // SFML 3.0: 座標もsf::Vector2fを渡すのが基本
    shape.setPosition({300.f, 200.f});

    while (window.isOpen()) {
        // SFML 3.0: pollEventはstd::optionalを返すようになりました
        while (const std::optional event = window.pollEvent()) {
            
            // イベントの種類を is<T> で判定するモダンなスタイル
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}
