#ifndef MENO_CONVERT_HPP
#define MENO_CONVERT_HPP

#include <meno/math/Color.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

/// 내부 전용 헤더. meno 타입 ↔ SFML 타입 변환을 한곳에 모아둔다.
///
/// 변환이 여기 한 파일에만 있어야 백엔드를 갈아끼울 때 손볼 곳이 명확해진다.
/// 다른 .cpp에서 sf:: 타입을 직접 조립하지 않도록 하자.
namespace meno::backend {

template <typename T>
[[nodiscard]] constexpr sf::Vector2<T> toSf(const Vec2<T>& v) noexcept {
    return {v.x, v.y};
}

template <typename T>
[[nodiscard]] constexpr Vec2<T> fromSf(const sf::Vector2<T>& v) noexcept {
    return {v.x, v.y};
}

template <typename T>
[[nodiscard]] constexpr sf::Rect<T> toSf(const Rect<T>& r) noexcept {
    return sf::Rect<T>{toSf(r.position), toSf(r.size)};
}

template <typename T>
[[nodiscard]] constexpr Rect<T> fromSf(const sf::Rect<T>& r) noexcept {
    return Rect<T>{fromSf(r.position), fromSf(r.size)};
}

[[nodiscard]] constexpr sf::Color toSf(const Color& c) noexcept {
    return sf::Color{c.r, c.g, c.b, c.a};
}

[[nodiscard]] constexpr Color fromSf(const sf::Color& c) noexcept {
    return Color{c.r, c.g, c.b, c.a};
}

/// meno의 각도는 전부 "도(degree), 시계 방향"이다.
/// SFML 3는 sf::Angle이라는 별도 타입을 쓰므로 여기서 감싼다.
[[nodiscard]] inline sf::Angle toSfAngle(float degrees) noexcept {
    return sf::degrees(degrees);
}

} // namespace meno::backend

#endif  // MENO_CONVERT_HPP