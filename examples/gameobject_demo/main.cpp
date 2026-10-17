// Scene -> GameObject -> Sprite 순서로 구성하는 정적인 예제.
// 상자 텍스처를 코드로 만들어 외부 이미지 파일 없이 실행할 수 있다.

#include <meno/core/Clock.hpp>
#include <meno/core/Window.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>
#include <meno/math/Color.hpp>
#include <meno/scene/Scene.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <utility>

namespace {

std::optional<meno::Texture> makeCrateTexture() {
    constexpr unsigned int size = 16;
    std::array<std::uint8_t, size * size * 4> pixels{};

    for (unsigned int y = 0; y < size; ++y) {
        for (unsigned int x = 0; x < size; ++x) {
            const bool frame = x < 2 || y < 2 || x >= size - 2 || y >= size - 2;
            const bool brace = x == y || x + y == size - 1;
            const bool seam = x == 5 || x == 10;
            const std::size_t index = (y * size + x) * 4;

            pixels[index + 0] = frame || brace ? 240 : (seam ? 130 : 180);
            pixels[index + 1] = frame || brace ? 185 : (seam ? 80 : 115);
            pixels[index + 2] = frame || brace ? 100 : (seam ? 45 : 60);
            pixels[index + 3] = 255;
        }
    }

    return meno::Texture::fromPixels({size, size}, pixels.data());
}

} // namespace

int main() {
    meno::Window window{{800u, 600u}, "meno - GameObject scene"};
    meno::Renderer renderer{window};

    auto texture = makeCrateTexture();
    if (!texture) {
        std::cerr << "Failed to create the crate texture.\n";
        return 1;
    }
    texture->setSmooth(false);

    // Scene이 객체를 소유한다. crate 참조는 객체가 삭제되기 전까지만 유효하다.
    meno::Scene scene;
    meno::GameObject& crate = scene.createGameObject();
    crate.transform().pos = {400.f, 300.f};
    crate.transform().magnitude = 10; // 16 x 16 텍스처를 160 x 160으로 표시

    // Sprite는 이미 기본 컴포넌트이므로 addComponent 대신 sprite()로 설정한다.
    crate.sprite().origin = {8.f, 8.f}; // 텍스처 중심을 객체 위치에 맞춘다.
    crate.sprite().setTexture(std::move(*texture));

    meno::Clock clock;
    clock.setFramerateLimit(60);
    scene.onEnter();

    while (window.isOpen()) {
        (void)clock.restart();
        window.pollEvents();
        if (!window.isOpen()) {
            break;
        }

        renderer.beginFrame(meno::Color::fromRgb(0x1E2430));

        scene.drawScene(renderer);
        renderer.endFrame();
    }

    scene.onExit();
    return 0;
}
