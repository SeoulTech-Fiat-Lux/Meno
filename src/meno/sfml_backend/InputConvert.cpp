#include "sfml_backend/InputConvert.hpp"

#include "sfml_backend/Convert.hpp"

namespace meno::backend {

// 키 이름을 하나씩 짝지었다. 두 열거형의 순서가 같더라도 static_cast로 바꾸지 않는다.
// SFML이 키를 추가하거나 순서를 바꾸면 조용히 다른 키로 읽히기 때문이다.
// 빠지거나 겹친 짝은 tests/input/test_input_convert.cpp가 잡는다.
Key fromSf(sf::Keyboard::Key key) noexcept {
    using K = sf::Keyboard::Key;
    // clang-format off
    switch (key) {
    case K::A: return Key::A;
    case K::B: return Key::B;
    case K::C: return Key::C;
    case K::D: return Key::D;
    case K::E: return Key::E;
    case K::F: return Key::F;
    case K::G: return Key::G;
    case K::H: return Key::H;
    case K::I: return Key::I;
    case K::J: return Key::J;
    case K::K: return Key::K;
    case K::L: return Key::L;
    case K::M: return Key::M;
    case K::N: return Key::N;
    case K::O: return Key::O;
    case K::P: return Key::P;
    case K::Q: return Key::Q;
    case K::R: return Key::R;
    case K::S: return Key::S;
    case K::T: return Key::T;
    case K::U: return Key::U;
    case K::V: return Key::V;
    case K::W: return Key::W;
    case K::X: return Key::X;
    case K::Y: return Key::Y;
    case K::Z: return Key::Z;
    case K::Num0: return Key::Num0;
    case K::Num1: return Key::Num1;
    case K::Num2: return Key::Num2;
    case K::Num3: return Key::Num3;
    case K::Num4: return Key::Num4;
    case K::Num5: return Key::Num5;
    case K::Num6: return Key::Num6;
    case K::Num7: return Key::Num7;
    case K::Num8: return Key::Num8;
    case K::Num9: return Key::Num9;
    case K::Escape: return Key::Escape;
    case K::LControl: return Key::LControl;
    case K::LShift: return Key::LShift;
    case K::LAlt: return Key::LAlt;
    case K::LSystem: return Key::LSystem;
    case K::RControl: return Key::RControl;
    case K::RShift: return Key::RShift;
    case K::RAlt: return Key::RAlt;
    case K::RSystem: return Key::RSystem;
    case K::Menu: return Key::Menu;
    case K::LBracket: return Key::LBracket;
    case K::RBracket: return Key::RBracket;
    case K::Semicolon: return Key::Semicolon;
    case K::Comma: return Key::Comma;
    case K::Period: return Key::Period;
    case K::Apostrophe: return Key::Apostrophe;
    case K::Slash: return Key::Slash;
    case K::Backslash: return Key::Backslash;
    case K::Grave: return Key::Grave;
    case K::Equal: return Key::Equal;
    case K::Hyphen: return Key::Hyphen;
    case K::Space: return Key::Space;
    case K::Enter: return Key::Enter;
    case K::Backspace: return Key::Backspace;
    case K::Tab: return Key::Tab;
    case K::PageUp: return Key::PageUp;
    case K::PageDown: return Key::PageDown;
    case K::End: return Key::End;
    case K::Home: return Key::Home;
    case K::Insert: return Key::Insert;
    case K::Delete: return Key::Delete;
    case K::Add: return Key::Add;
    case K::Subtract: return Key::Subtract;
    case K::Multiply: return Key::Multiply;
    case K::Divide: return Key::Divide;
    case K::Left: return Key::Left;
    case K::Right: return Key::Right;
    case K::Up: return Key::Up;
    case K::Down: return Key::Down;
    case K::Numpad0: return Key::Numpad0;
    case K::Numpad1: return Key::Numpad1;
    case K::Numpad2: return Key::Numpad2;
    case K::Numpad3: return Key::Numpad3;
    case K::Numpad4: return Key::Numpad4;
    case K::Numpad5: return Key::Numpad5;
    case K::Numpad6: return Key::Numpad6;
    case K::Numpad7: return Key::Numpad7;
    case K::Numpad8: return Key::Numpad8;
    case K::Numpad9: return Key::Numpad9;
    case K::F1: return Key::F1;
    case K::F2: return Key::F2;
    case K::F3: return Key::F3;
    case K::F4: return Key::F4;
    case K::F5: return Key::F5;
    case K::F6: return Key::F6;
    case K::F7: return Key::F7;
    case K::F8: return Key::F8;
    case K::F9: return Key::F9;
    case K::F10: return Key::F10;
    case K::F11: return Key::F11;
    case K::F12: return Key::F12;
    case K::F13: return Key::F13;
    case K::F14: return Key::F14;
    case K::F15: return Key::F15;
    case K::Pause: return Key::Pause;
    case K::Unknown: return Key::Unknown;
    }
    // clang-format on
    return Key::Unknown;
}

std::optional<MouseButton> fromSf(sf::Mouse::Button button) noexcept {
    using B = sf::Mouse::Button;
    switch (button) {
    case B::Left:
        return MouseButton::Left;
    case B::Right:
        return MouseButton::Right;
    case B::Middle:
        return MouseButton::Middle;
    case B::Extra1:
        return MouseButton::Extra1;
    case B::Extra2:
        return MouseButton::Extra2;
    }
    return std::nullopt;
}

std::optional<Event> fromSf(const sf::Event& event) noexcept {
    if (event.is<sf::Event::FocusLost>()) {
        return event::FocusLost{};
    }
    if (const auto* e = event.getIf<sf::Event::KeyPressed>()) {
        return event::KeyPressed{fromSf(e->code)};
    }
    if (const auto* e = event.getIf<sf::Event::KeyReleased>()) {
        return event::KeyReleased{fromSf(e->code)};
    }
    if (const auto* e = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (const std::optional button = fromSf(e->button)) {
            return event::MouseButtonPressed{*button, fromSf(e->position)};
        }
        return std::nullopt;
    }
    if (const auto* e = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (const std::optional button = fromSf(e->button)) {
            return event::MouseButtonReleased{*button, fromSf(e->position)};
        }
        return std::nullopt;
    }
    if (const auto* e = event.getIf<sf::Event::MouseMoved>()) {
        return event::MouseMoved{fromSf(e->position)};
    }
    if (const auto* e = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (e->wheel != sf::Mouse::Wheel::Vertical) {
            return std::nullopt;
        }
        return event::MouseWheelScrolled{e->delta, fromSf(e->position)};
    }
    if (const auto* e = event.getIf<sf::Event::TextEntered>()) {
        return event::TextEntered{e->unicode};
    }
    return std::nullopt;
}

} // namespace meno::backend
