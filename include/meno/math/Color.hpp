#ifndef MENO_COLOR_HPP
#define MENO_COLOR_HPP

#include <cstdint>
#include <type_traits>

namespace meno {

/// 8비트 RGBA 색상계.
/// 계산의 편의성을 위해 색상의 기본값은 불투명 흰색으로 설정한다.
/// c.f. 결과 = 원본색 * tint.
/// 이때 흰색은 곱셉의 항등원으로 작용하며, tint를 흰색으로 설정하면 원본색이 그대로 유지된다.
struct Color {
    std::uint8_t r{255};
    std::uint8_t g{255};
    std::uint8_t b{255};
    std::uint8_t a{255};

    constexpr Color() = default;
    constexpr Color(std::uint8_t inR, std::uint8_t inG, std::uint8_t inB, std::uint8_t inA = 255)
        : r(inR), g(inG), b(inB), a(inA) {}

    /// 0xRRGGBBAA 형태의 리터럴(8자리 16진수)에서 Color 객체를 생성하는 정적 멤버 함수.
    /// 알파까지 8자리를 모두 적어야 하며, 웹에서 흔한 6자리(0x1E2430)를 넣으면
    /// 0x001E2430으로 읽혀 R=0x00, G=0x1E, B=0x24, A=0x30이 된다. (색이 한 칸씩 밀리고 거의
    /// 투명해진다.) 6자리는 fromRgb를 쓴다. 두 리터럴 모두 std::uint32_t라 오버로드로는 구분할 수
    /// 없어서 이름을 나눴다.
    [[nodiscard]] static constexpr Color fromRgba(std::uint32_t rgba) {
        return Color{static_cast<std::uint8_t>((rgba >> 24 & 0xFF)),
                     static_cast<std::uint8_t>((rgba >> 16 & 0xFF)),
                     static_cast<std::uint8_t>((rgba >> 8 & 0xFF)),
                     static_cast<std::uint8_t>(rgba & 0xFF)};
    }

    /// 0xRRGGBB 형태(6자리)의 리터럴에서 불투명한 Color 객체를 생성하는 정적 멤버 함수.
    /// 6자리를 넘는 값(8자리 0xRRGGBBAA를 잘못 넣은 경우 등)은 빌드 설정과 상관없이 아래처럼
    /// 처리한다: 상수 식(constexpr 변수, static_assert)에서는 컴파일 에러가 나며, 그 밖의 경우에는
    /// 마젠타(0xFF00FF)를 돌려준다. 텍스처가 없을 때처럼 화면에서 바로 눈에 띄는 '잘못된 색'을
    /// 표시할 용도이다. 앞자리가 00인 8자리(0x00000080 등)는 값이 6자리 범위라 어떤 검사로도 잡을
    /// 수 없다. 반투명은 withAlpha나 fromRgba로 만든다.
    [[nodiscard]] static constexpr Color fromRgb(std::uint32_t rgb) {
        if (rgb > 0xFFFFFFu) {
            if (std::is_constant_evaluated()) {
                throw "fromRgb() takes 0xRRGGBB; use fromRgba() for 0xRRGGBBAA";
            }
            return Color{255, 0, 255};
        }
        return Color{static_cast<std::uint8_t>((rgb >> 16 & 0xFF)),
                     static_cast<std::uint8_t>((rgb >> 8 & 0xFF)),
                     static_cast<std::uint8_t>(rgb & 0xFF)};
    }

    [[nodiscard]] constexpr Color withAlpha(std::uint8_t alpha) const {
        return Color{r, g, b, alpha};
    }
};

constexpr bool operator==(const Color& lhs, const Color& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

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
