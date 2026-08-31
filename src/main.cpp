//
// Created by 최상준 on 26. 8. 6..
//


#include <SFML/Graphics.hpp>
#include <meno/core/Clock.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "GameObjectDemo");
    meno::Clock clock;
    clock.setFramerateLimit(60);

    while (window.isOpen()) {
        (void)clock.restart();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color(30, 30, 30));
        // TODO: GameObject / Transform / Collider 구현 후 여기서 테스트
        window.display();
    }

    return 0;
}
