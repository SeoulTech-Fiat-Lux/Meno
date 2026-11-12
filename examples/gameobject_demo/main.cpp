// Scene -> GameObject -> Sprite 순서로 구성하는 정적인 예제.
// 상자 텍스처를 코드로 만들어 외부 이미지 파일 없이 실행할 수 있다.

#include <meno/Meno.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <stdexcept>
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

class GameObjectDemo final : public meno::Game {
public:
    GameObjectDemo()
        : Game({.application = {.framerateLimit = 60},
                .windowSize = {800u, 600u},
                .title = "meno - GameObject scene",
                .clearColor = meno::Color::fromRgb(0x1E2430)}) {}

private:
    void onStart() override {
        auto texture = makeCrateTexture();
        if (!texture) {
            throw std::runtime_error("Failed to create the crate texture");
        }
        texture->setSmooth(false);

        auto& scene = scenes().createScene();
        meno::GameObject& crate = scene.createGameObject();
        crate.transform().pos = {400.f, 300.f};
        crate.transform().magnitude = 10;
        crate.sprite().origin = {8.f, 8.f};
        crate.sprite().setTexture(std::move(*texture));

        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("Failed to activate GameObject scene");
        }
    }
};

} // namespace

int main() {
    try {
        GameObjectDemo game;
        game.run();
    } catch (const std::exception& error) {
        std::cerr << "GameObject demo: " << error.what() << '\n';
        return 1;
    }
}
