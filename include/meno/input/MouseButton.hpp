#ifndef MENO_MOUSE_BUTTON_HPP
#define MENO_MOUSE_BUTTON_HPP

#include <cstddef>
#include <cstdint>

namespace meno {

// 마우스 버튼. Extra1과 Extra2는 마우스 측면의 앞뒤 이동 버튼과 같다.
enum class MouseButton : std::uint8_t {
    Left,
    Right,
    Middle,
    Extra1,
    Extra2,
};

// MouseButton 열거형의 값 개수. Key.hpp와 마찬가지로 버튼별 상태 배열을 만들 때 그 크기로 활용한다.
inline constexpr std::size_t mouseButtonCount = static_cast<std::size_t>(MouseButton::Extra2) + 1;

} // namespace meno

#endif // MENO_MOUSE_BUTTON_HPP