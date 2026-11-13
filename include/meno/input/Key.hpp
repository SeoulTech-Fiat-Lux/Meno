#ifndef MENO_KEY_HPP
#define MENO_KEY_HPP

#include <cstddef>
#include <cstdint>

namespace meno {

/// 키보드 키입니다. 키캡에 적힌 글자, 즉 자판 배열을 따릅니다.
///
/// 예를 들어 AZERTY 자판에서는 QWERTY의 Q 자리에 있는 키를 누르면 Key::A가 들어옵니다.
/// 이름이 헷갈리는 키: Num0~Num9는 자판 위쪽 줄의 숫자, Numpad0~Numpad9는 숫자 패드입니다.
/// LSystem·RSystem은 Windows 키(macOS는 Command), Grave는 숫자 1 왼쪽의 키, Hyphen은 - 키,
/// Equal은 = 키입니다.
///
/// 여기에 없는 키는 Key::Unknown으로 들어옵니다. Input은 Unknown을 추적하지 않으므로
/// isDown(Key::Unknown)은 항상 false입니다.
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

/// Key 값의 개수(Unknown 포함)입니다. 키별 상태 배열의 크기로 씁니다.
inline constexpr std::size_t keyCount = static_cast<std::size_t>(Key::Pause) + 1;

} // namespace meno

#endif // MENO_KEY_HPP
