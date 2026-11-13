
#include <meno/core/Clock.hpp>
#include <SFML/Graphics.hpp>

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
        window.display();
    }

    return 0;
}
