#ifndef MENO_COLOR_HPP
#define MENO_COLOR_HPP

#include <cstdint>

namespace meno {

// 8비트 RGBA 색상계.
// 계산의 편의성을 위해 색상의 기본값은 불투명 흰색으로 설정한다.
// c.f. 결과 = 원본색 * tint.
// 이때 흰색은 곱셉의 항등원으로 작용하며, tint를 흰색으로 설정하면 원본색이 그대로 유지된다.
struct Color {
    // uint8_t 타입(8비트짜리 부호 없는 정수)의 r(빨강), g(초록), b(파랑), a(알파) 멤버를 가지며, 기본값은 모두 255로 설정되어 있다.
    std::uint8_t r{255};
    std::uint8_t g{255};
    std::uint8_t b{255};
    std::uint8_t a{255};

    // 생성자들. Color 객체를 다양한 방식으로 초기화할 수 있도록 제공.
    constexpr Color() = default; // 기본 생성자. Color 객체를 기본값으로 초기화.
    constexpr Color(std::uint8_t inR, std::uint8_t inG, std::uint8_t inB, std::uint8_t inA = 255)
        : r(inR), g(inG), b(inB), a(inA) {} // r, g, b, a 값을 인자로 받아 Color 객체를 초기화.

    // 0xRRGGBBAA 형태의 리터럴에서 Color 객체를 생성하는 정적 멤버 함수.
    [[nodiscard]] static constexpr Color fromHex(std::uint32_t rgba) {
        // 비트 마스킹과 시프트 연산을 통해 32비트 정수에서 각 색상 성분을 추출하여 Color 객체를 생성한다.
        return Color{static_cast<std::uint8_t>((rgba >> 24 & 0xFF)), // 상위 8비트(R) 추출
                     static_cast<std::uint8_t>((rgba >> 16 & 0xFF)), // 중간 8비트(G) 추출
                     static_cast<std::uint8_t>((rgba >> 8 & 0xFF)),  // 하위 8비트(B) 추출
                     static_cast<std::uint8_t>(rgba & 0xFF)};        // 알파 값 추출
    }

    // 현재 Color 객체의 RGB 값은 그대로 유지하면서, 알파 값만 변경된 새로운 Color 객체를 반환.
    [[nodiscard]] constexpr Color withAlpha(std::uint8_t alpha) const {
        return Color{r, g, b, alpha};
    }
};

// Color 객체 간의 동등성을 비교하는 전역 연산자 오버로딩.
constexpr bool operator==(const Color& lhs, const Color& rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

// 이름 있는 색상들을 별도의 네임스페이스에 정의하여, 코드에서 쉽게 접근할 수 있도록 한다.
// 클래스의 static 멤버로는 컴파일 타임에 정의가 되지 않은 자기 자신의 타입을 담을 수 없기 때문에 별도의 네임스페이스를 활용한다.
namespace colors {

inline constexpr Color White{255, 255, 255}; // 흰색
inline constexpr Color Black{0, 0, 0};       // 검은색
inline constexpr Color Red{255, 0, 0};       // 빨간색
inline constexpr Color Green{0, 255, 0};     // 초록색
inline constexpr Color Blue{0, 0, 255};      // 파란색
inline constexpr Color Yellow{255, 255, 0};   // 노란색
inline constexpr Color Magenta{255, 0, 255};  // 마젠타
inline constexpr Color Cyan{0, 255, 255};     // 시안
inline constexpr Color Gray{128, 128, 128};   // 회색
inline constexpr Color Transparent{0, 0, 0, 0}; // 투명

} // namespace colors

} // namespace meno

#endif  // MENO_COLOR_HPP