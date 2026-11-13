#ifndef MENO_MOUSE_BUTTON_HPP
#define MENO_MOUSE_BUTTON_HPP

#include <cstddef>
#include <cstdint>

namespace meno {

/// 마우스 버튼입니다. Extra1과 Extra2는 마우스 옆면의 보조 버튼(보통 뒤로·앞으로)입니다.
enum class MouseButton : std::uint8_t {
    Left,
    Right,
    Middle,
    Extra1,
    Extra2,
};

/// MouseButton 값의 개수입니다. 버튼별 상태 배열의 크기로 씁니다.
inline constexpr std::size_t mouseButtonCount = static_cast<std::size_t>(MouseButton::Extra2) + 1;

} // namespace meno

#endif // MENO_MOUSE_BUTTON_HPP
