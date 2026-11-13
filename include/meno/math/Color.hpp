#ifndef MENO_COLOR_HPP
#define MENO_COLOR_HPP

#include <cstdint>
#include <type_traits>

namespace meno {

/// 8비트 RGBA 색.
/// 기본값은 불투명 흰색이다. 그리는 결과는 원본색 × tint이므로, 흰색이 곱셈의 항등원 역할을 한다.
struct Color {
    std::uint8_t r{255};
    std::uint8_t g{255};
    std::uint8_t b{255};
    std::uint8_t a{255};

    constexpr Color() = default;
    constexpr Color(std::uint8_t inR, std::uint8_t inG, std::uint8_t inB, std::uint8_t inA = 255)
        : r(inR), g(inG), b(inB), a(inA) {}

    constexpr bool operator==(const Color&) const = default;

    /// 0xRRGGBBAA(8자리)에서 만든다.
    /// 웹에서 흔한 6자리(0x1E2430)를 넣으면 0x001E2430으로 읽혀
    /// R=0x00, G=0x1E, B=0x24, A=0x30이 된다(색이 한 칸씩 밀리고 거의 투명해진다).
    /// 6자리는 fromRgb를 쓴다. 두 리터럴 모두 std::uint32_t라 오버로드로는 구분할 수 없어서
    /// 이름을 나눴다.
    [[nodiscard]] static constexpr Color fromRgba(std::uint32_t rgba) {
        return Color{static_cast<std::uint8_t>(rgba >> 24 & 0xFF),
                     static_cast<std::uint8_t>(rgba >> 16 & 0xFF),
                     static_cast<std::uint8_t>(rgba >> 8 & 0xFF),
                     static_cast<std::uint8_t>(rgba & 0xFF)};
    }

    /// 0xRRGGBB(6자리)에서 불투명한 색을 만든다.
    /// 6자리를 넘는 값(8자리를 잘못 넣은 경우 등)은 상수식(constexpr 변수, static_assert)에서는
    /// 컴파일 에러가 나고, 그 밖에서는 화면에서 바로 눈에 띄도록 마젠타를 돌려준다.
    /// 앞자리가 00인 8자리(0x00000080 등)는 값이 6자리 범위라 어떤 검사로도 잡을 수 없다.
    /// 반투명은 withAlpha나 fromRgba로 만든다.
    [[nodiscard]] static constexpr Color fromRgb(std::uint32_t rgb) {
        if (rgb > 0xFFFFFFu) {
            if (std::is_constant_evaluated()) {
                // 상수식 안에서 throw를 만나면 컴파일 에러가 되고, 이 문장이 메시지로 보인다.
                throw "fromRgb() takes 0xRRGGBB; use fromRgba() for 0xRRGGBBAA";
            }
            // colors::Magenta는 아직 선언 전이라 쓸 수 없다.
            return Color{255, 0, 255};
        }
        return Color{static_cast<std::uint8_t>(rgb >> 16 & 0xFF),
                     static_cast<std::uint8_t>(rgb >> 8 & 0xFF),
                     static_cast<std::uint8_t>(rgb & 0xFF)};
    }

    [[nodiscard]] constexpr Color withAlpha(std::uint8_t alpha) const {
        return Color{r, g, b, alpha};
    }
};

/// 자주 쓰는 색.
/// Color의 static 멤버로는 아직 완성되지 않은 자기 타입을 constexpr로 담을 수 없어서
/// 네임스페이스로 둔다.
namespace colors {

inline constexpr Color White{255, 255, 255};
inline constexpr Color Black{0, 0, 0};
inline constexpr Color Red{255, 0, 0};
inline constexpr Color Green{0, 255, 0};
inline constexpr Color Blue{0, 0, 255};
inline constexpr Color Yellow{255, 255, 0};
inline constexpr Color Magenta{255, 0, 255};
inline constexpr Color Cyan{0, 255, 255};
inline constexpr Color Gray{128, 128, 128};
inline constexpr Color Transparent{0, 0, 0, 0};

} // namespace colors

} // namespace meno

#endif // MENO_COLOR_HPP
