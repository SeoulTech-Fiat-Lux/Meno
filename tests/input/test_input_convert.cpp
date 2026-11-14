#include <meno/input/Event.hpp>
#include <meno/input/Key.hpp>
#include <meno/input/MouseButton.hpp>

#include "sfml_backend/InputConvert.hpp"

#include "check.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <cstdio>
#include <optional>
#include <variant>

namespace {

using meno::Key;
using meno::MouseButton;
using meno::backend::fromSf;
namespace ev = meno::event;

void testKeys() {
    // Unknown을 뺀 키(A~Pause)는 sf::Keyboard::Key와 meno::Key(Key.hpp)의 순서가 같다.
    // 따라서 변환 결과의 번호가 "SFML 번호 + 1"이라면 모든 키가 알맞게 짝지어진 것이다.
    // 이 검사는 이웃한 두 키가 뒤바뀐 경우까지 잡도록 설계되어 있다.
    MENO_CHECK(sf::Keyboard::KeyCount == meno::keyCount - 1);

    bool allMatch = true;
    for (unsigned int code = 0; code < sf::Keyboard::KeyCount; ++code) {
        const Key key = fromSf(static_cast<sf::Keyboard::Key>(code));
        if (static_cast<unsigned int>(key) != code + 1) {
            std::printf("FAIL  sf::Keyboard::Key %u -> meno::Key %u\n", code,
                        static_cast<unsigned int>(key));
            allMatch = false;
        }
    }
    MENO_CHECK(allMatch);

    using K = sf::Keyboard::Key;
    MENO_CHECK(fromSf(K::Unknown) == Key::Unknown);
    MENO_CHECK(fromSf(K::A) == Key::A);
    MENO_CHECK(fromSf(K::Pause) == Key::Pause);
}

void testMouseButtons() {
    MENO_CHECK(sf::Mouse::ButtonCount == meno::mouseButtonCount);
    MENO_CHECK(fromSf(sf::Mouse::Button::Left) == MouseButton::Left);
    MENO_CHECK(fromSf(sf::Mouse::Button::Right) == MouseButton::Right);
    MENO_CHECK(fromSf(sf::Mouse::Button::Middle) == MouseButton::Middle);
    MENO_CHECK(fromSf(sf::Mouse::Button::Extra1) == MouseButton::Extra1);
    MENO_CHECK(fromSf(sf::Mouse::Button::Extra2) == MouseButton::Extra2);
}

void testEvents() {
    const auto pressed = fromSf(sf::Event{sf::Event::KeyPressed{.code = sf::Keyboard::Key::Space}});
    const auto* key = pressed ? std::get_if<ev::KeyPressed>(&*pressed) : nullptr;
    MENO_CHECK(key != nullptr && key->key == Key::Space);

    const auto released = fromSf(sf::Event{sf::Event::KeyReleased{.code = sf::Keyboard::Key::Q}});
    const auto* keyUp = released ? std::get_if<ev::KeyReleased>(&*released) : nullptr;
    MENO_CHECK(keyUp != nullptr && keyUp->key == Key::Q);

    const auto click =
        fromSf(sf::Event{sf::Event::MouseButtonPressed{sf::Mouse::Button::Right, {12, 34}}});
    const auto* button = click ? std::get_if<ev::MouseButtonPressed>(&*click) : nullptr;
    MENO_CHECK(button != nullptr && button->button == MouseButton::Right &&
               button->position == meno::Vec2i(12, 34));

    const auto unclick =
        fromSf(sf::Event{sf::Event::MouseButtonReleased{sf::Mouse::Button::Middle, {5, 6}}});
    const auto* buttonUp = unclick ? std::get_if<ev::MouseButtonReleased>(&*unclick) : nullptr;
    MENO_CHECK(buttonUp != nullptr && buttonUp->button == MouseButton::Middle &&
               buttonUp->position == meno::Vec2i(5, 6));

    const auto moved = fromSf(sf::Event{sf::Event::MouseMoved{{7, 8}}});
    const auto* move = moved ? std::get_if<ev::MouseMoved>(&*moved) : nullptr;
    MENO_CHECK(move != nullptr && move->position == meno::Vec2i(7, 8));

    const auto wheel =
        fromSf(sf::Event{sf::Event::MouseWheelScrolled{sf::Mouse::Wheel::Vertical, -1.5f, {1, 2}}});
    const auto* scroll = wheel ? std::get_if<ev::MouseWheelScrolled>(&*wheel) : nullptr;
    MENO_CHECK(scroll != nullptr && scroll->delta == -1.5f &&
               scroll->position == meno::Vec2i(1, 2));

    const auto text = fromSf(sf::Event{sf::Event::TextEntered{U'\uD55C'}});
    const auto* entered = text ? std::get_if<ev::TextEntered>(&*text) : nullptr;
    MENO_CHECK(entered != nullptr && entered->unicode == U'\uD55C');

    const auto focus = fromSf(sf::Event{sf::Event::FocusLost{}});
    MENO_CHECK(focus.has_value() && std::holds_alternative<ev::FocusLost>(*focus));

    // Input이 다루지 않는 이벤트는 변환하지 않고 std::nullopt를 돌려준다.
    MENO_CHECK(!fromSf(sf::Event{sf::Event::Closed{}}).has_value());
    MENO_CHECK(!fromSf(sf::Event{sf::Event::FocusGained{}}).has_value());
    MENO_CHECK(!fromSf(sf::Event{sf::Event::Resized{{800u, 600u}}}).has_value());
    MENO_CHECK(
        !fromSf(sf::Event{sf::Event::MouseWheelScrolled{sf::Mouse::Wheel::Horizontal, 1.f, {}}})
             .has_value());
}

} // namespace

int main() {
    testKeys();
    testMouseButtons();
    testEvents();
    return meno::test::report();
}
