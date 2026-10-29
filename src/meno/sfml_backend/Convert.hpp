#ifndef MENO_CONVERT_HPP
#define MENO_CONVERT_HPP

#include <meno/math/Camera2D.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

// 내부 전용 헤더. meno 타입 ↔ SFML 타입 변환을 한곳에 모아둔다.
//
// 변환이 여기 한 파일에만 있어야 백엔드를 갈아끼울 때 손볼 곳이 명확해진다.
// 다른 .cpp에서 sf:: 타입을 직접 조립하지 않도록 하자.
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

// meno의 각도는 전부 "도(degree), 시계 방향"이다.
// SFML 3는 sf::Angle이라는 별도 타입을 쓰므로 여기서 감싼다.
[[nodiscard]] inline sf::Angle toSfAngle(float degrees) noexcept {
    return sf::degrees(degrees);
}

// Camera2D를 sf::View로 바꾸는 유일한 지점. framebuffer는 렌더 타깃의 현재 픽셀 크기다.
//
// 크기, 회전, 뷰포트를 전부 math/Camera2D.hpp의 함수로 계산해서 넘긴다. SFML이 따로 계산하게 두면
// 그리는 결과와 Renderer::worldToScreen 같은 좌표 계산이 서로 다른 규칙을 따르게 된다.
// 두 쪽이 일치하는지는 tests/graphics/test_camera_parity.cpp가 검사한다.
[[nodiscard]] inline sf::View toSfView(const Camera2D& camera, Vec2u framebuffer) {
    sf::View view{toSf(camera.center), toSf(viewRect(camera, framebuffer).size)};

    // 360으로 접은 각도를 넘긴다. 큰 각도를 그대로 넘기면 SFML이 라디안으로 바꾸는 단계에서 정밀도를 잃어
    // 화면에 그려진 위치가 좌표 계산 결과와 몇 픽셀씩 어긋난다.
    view.setRotation(toSfAngle(detail::wrapDegrees(camera.rotation)));

    // SFML은 비율 뷰포트에 창 크기를 곱한 뒤 위치와 크기를 따로 반올림한다.
    // 모서리 기준으로 반올림해 둔 픽셀 사각형을 다시 비율로 바꿔 넘기면, SFML이 반올림해도 정확히 같은 정수가 나온다.
    // 창 크기가 0이면 비율로 바꿀 수 없다. 어차피 그릴 픽셀이 없으므로 기본 뷰포트를 그대로 둔다.
    if (framebuffer.x > 0 && framebuffer.y > 0) {
        const Rectf pixels = viewportPixels(camera, framebuffer);
        const auto screen = static_cast<Vec2f>(framebuffer);
        view.setViewport(sf::FloatRect{{pixels.left() / screen.x, pixels.top() / screen.y},
                                       {pixels.size.x / screen.x, pixels.size.y / screen.y}});
    }
    return view;
}

} // namespace meno::backend

#endif  // MENO_CONVERT_HPP