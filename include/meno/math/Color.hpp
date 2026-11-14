#ifndef MENO_COLOR_HPP
#define MENO_COLOR_HPP

#include <cstdint>
#include <type_traits>

namespace meno {

/// \addtogroup math
/// \{

/// 8비트 RGBA 색입니다.
///
/// 기본값은 불투명 흰색입니다. tint로 쓰면 원본 색에 곱해지므로, 흰색은 원본을 그대로 둡니다.
///
/// \code{.cpp}
/// const meno::Color orange{255, 128, 0};
/// const meno::Color background = meno::Color::fromRgb(0x1E2430);
/// const meno::Color shadow = meno::colors::Black.withAlpha(128);
/// \endcode
///
/// \see colors
struct Color {
    std::uint8_t r{255}; ///< 빨강(0~255)
    std::uint8_t g{255}; ///< 초록(0~255)
    std::uint8_t b{255}; ///< 파랑(0~255)
    std::uint8_t a{255}; ///< 불투명도. 0은 완전히 투명하고 255는 불투명합니다.

    /// 불투명 흰색을 만듭니다.
    constexpr Color() = default;
    /// 채널 값으로 만듭니다. a를 생략하면 불투명(255)입니다.
    constexpr Color(std::uint8_t inR, std::uint8_t inG, std::uint8_t inB, std::uint8_t inA = 255)
        : r(inR), g(inG), b(inB), a(inA) {}

    /// 네 채널이 모두 같으면 같은 색입니다.
    constexpr bool operator==(const Color&) const = default;

    // 6자리와 8자리 리터럴이 모두 std::uint32_t라 오버로드로는 구분할 수 없어서
    // fromRgb와 이름을 나눴다.
    /// 0xRRGGBBAA(8자리)에서 만듭니다.
    ///
    /// 웹에서 흔한 6자리(0x1E2430)를 넣으면 0x001E2430으로 읽혀서
    /// R=0x00, G=0x1E, B=0x24, A=0x30이 됩니다(색이 한 칸씩 밀리고 거의 투명해집니다).
    /// 6자리 값에는 fromRgb()를 씁니다.
    [[nodiscard]] static constexpr Color fromRgba(std::uint32_t rgba) {
        return Color{static_cast<std::uint8_t>(rgba >> 24 & 0xFF),
                     static_cast<std::uint8_t>(rgba >> 16 & 0xFF),
                     static_cast<std::uint8_t>(rgba >> 8 & 0xFF),
                     static_cast<std::uint8_t>(rgba & 0xFF)};
    }

    /// 0xRRGGBB(6자리)에서 불투명한 색을 만듭니다.
    ///
    /// 6자리를 넘는 값(8자리를 잘못 넣은 경우 등)을 넣으면 상수식(constexpr 변수,
    /// static_assert)에서는 컴파일 에러가 나고, 그 밖에서는 화면에서 바로 눈에 띄도록 마젠타를
    /// 돌려줍니다. 앞자리가 00인 8자리(0x00000080 등)는 값이 6자리 범위 안에 있으므로 어떤
    /// 검사로도 잡을 수 없습니다. 반투명한 색은 withAlpha()나 fromRgba()로 만듭니다.
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

    /// 불투명도만 alpha로 바꾼 색을 돌려줍니다. 원래 색은 바뀌지 않습니다.
    [[nodiscard]] constexpr Color withAlpha(std::uint8_t alpha) const {
        return Color{r, g, b, alpha};
    }
};

// Color의 static 멤버로는 아직 완성되지 않은 자기 타입을 constexpr로 담을 수 없어서
// 네임스페이스로 둔다.
/// 자주 쓰는 색입니다.
namespace colors {

/// \addtogroup math
/// \{

inline constexpr Color White{255, 255, 255};    ///< 흰색
inline constexpr Color Black{0, 0, 0};          ///< 검은색
inline constexpr Color Red{255, 0, 0};          ///< 빨간색
inline constexpr Color Green{0, 255, 0};        ///< 초록색
inline constexpr Color Blue{0, 0, 255};         ///< 파란색
inline constexpr Color Yellow{255, 255, 0};     ///< 노란색
inline constexpr Color Magenta{255, 0, 255};    ///< 자홍색. fromRgb() 오류 시에도 이 색입니다.
inline constexpr Color Cyan{0, 255, 255};       ///< 청록색
inline constexpr Color Gray{128, 128, 128};     ///< 회색
inline constexpr Color Transparent{0, 0, 0, 0}; ///< 완전히 투명한 색

/// \}

} // namespace colors

/// \}

} // namespace meno

#endif // MENO_COLOR_HPP
