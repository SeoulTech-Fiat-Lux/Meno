#ifndef MENO_KEY_HPP
#define MENO_KEY_HPP

#include <cstddef>
#include <cstdint>

namespace meno {

/// 키캡에 적힌 글자 즉, 자판 배열을 따르는 키보드 키.
/// ex. AZERTY 자판에서 Key::A는 QWERTY 자판에서 Key::Q에 해당한다.
enum class Key : std::uint8_t {
    Unknown,
    // clang-format off
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Escape,
    LControl, LShift, LAlt, LSystem,
    RControl, RShift, RAlt, RSystem,
    Menu,
    LBracket, RBracket, Semicolon, Comma, Period, Apostrophe,
    Slash, Backslash, Grave, Equal, Hyphen,
    Space, Enter, Backspace, Tab,
    PageUp, PageDown, End, Home, Insert, Delete,
    Add, Subtract, Multiply, Divide,
    Left, Right, Up, Down,
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
    Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15,
    Pause,
    // clang-format on
};

/// Unknown을 포함한 Key 열거형의 값 개수. 키별 상태 배열을 만들 때 그 크기로 활용한다.
inline constexpr std::size_t keyCount = static_cast<std::size_t>(Key::Pause) + 1;

} // namespace meno

#endif // MENO_KEY_HPP
