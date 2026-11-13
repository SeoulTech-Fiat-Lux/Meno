#ifndef MENO_TEST_PIXELS_HPP
#define MENO_TEST_PIXELS_HPP

// 그린 픽셀을 읽는 테스트 헬퍼.
// 내부 헤더(sfml_backend/Access.hpp)로 sf::RenderWindow를 꺼내므로, CMake에서 src/meno 경로와
// SFML::Graphics를 직접 연결한 테스트만 include할 수 있다. 예: render_resize, render_shapes

#include <meno/window/Window.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Vec2.hpp>

#include "sfml_backend/Access.hpp"
#include "sfml_backend/MathConvert.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace meno::test {

// 지금까지 그린 백 버퍼를 읽는다. endFrame(= display)으로 버퍼를 바꾸기 전에 불러야 한다.
[[nodiscard]] inline sf::Image capture(Window& window) {
    sf::RenderWindow& native = backend::WindowAccess::native(window);
    sf::Texture texture{native.getSize()};
    texture.update(native);
    return texture.copyToImage();
}

// 점 point가 들어 있는 픽셀의 색이 color인지를 확인. 창 밖의 점은 늘 false다.
// getPixel은 범위를 넘으면 Debug에서는 assert로 죽고, Release에서는 엉뚱한 메모리를 읽는다.
[[nodiscard]] inline bool isColor(const sf::Image& image, Vec2f point, Color color) {
    const sf::Vector2u size = image.getSize();
    if (!(point.x >= 0.f && point.y >= 0.f && point.x < static_cast<float>(size.x) &&
          point.y < static_cast<float>(size.y))) {
        return false;
    }
    const sf::Vector2u pixel{static_cast<unsigned int>(point.x),
                             static_cast<unsigned int>(point.y)};
    return image.getPixel(pixel) == backend::toSf(color);
}

// 픽셀 좌표로 묻는 판. 범위 검사는 위 함수가 한다.
[[nodiscard]] inline bool isColor(const sf::Image& image, unsigned int x, unsigned int y,
                                  Color color) {
    return isColor(image, Vec2f{static_cast<float>(x), static_cast<float>(y)}, color);
}

} // namespace meno::test

#endif // MENO_TEST_PIXELS_HPP
