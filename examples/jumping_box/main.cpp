// 기존 공개 API만 사용한다. 중력과 착지 반응은 이 예제의 게임 로직이다.
#include <meno/Meno.hpp>
#include <meno/scene/SceneManager.hpp>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

constexpr unsigned int cratePixels = 16;
constexpr unsigned int floorWidthPixels = 192;
constexpr unsigned int floorHeightPixels = 16;
constexpr float scale = 4.f;
constexpr float crateSize = cratePixels * scale;
constexpr float floorWidth = floorWidthPixels * scale;
constexpr float floorX = 96.f;
constexpr float floorY = 432.f;
constexpr float speed = 180.f;
constexpr float gravity = 1400.f;
constexpr float jumpSpeed = 620.f;
constexpr double jumpPause = 0.18;

meno::Texture makeTexture(bool floor) {
    const unsigned int width = floor ? floorWidthPixels : cratePixels;
    const unsigned int height = floor ? floorHeightPixels : cratePixels;
    std::vector<std::uint8_t> pixels(width * height * 4);
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            meno::Color color;
            if (floor) {
                const bool speckle = (x * 7 + y * 11) % 23 < 3;
                color = y < 3 ? meno::Color::fromRgb(0x79C88C) :
                    (speckle ? meno::Color::fromRgb(0x44566A) : meno::Color::fromRgb(0x334155));
            } else {
                const bool rim = x < 2 || y < 2 || x >= width - 2 || y >= height - 2;
                const bool brace = x == y || x + y == width - 1;
                color = rim ? meno::Color::fromRgb(0xFFD18A) :
                    (brace ? meno::Color::fromRgb(0xEAAF62) : meno::Color::fromRgb(0xAC683D));
            }
            const auto index = (y * width + x) * 4;
            pixels[index] = color.r;
            pixels[index + 1] = color.g;
            pixels[index + 2] = color.b;
            pixels[index + 3] = 255;
        }
    }
    auto texture = meno::Texture::fromPixels({width, height}, pixels.data());
    if (!texture) throw std::runtime_error("Failed to create demo texture");
    texture->setSmooth(false);
    return std::move(*texture);
}

class JumpingBoxDemo final : public meno::Game {
public:
    JumpingBoxDemo()
        : Game({.application = {.framerateLimit = 60},
                .windowSize = {960u, 600u},
                .title = "meno - Jumping box",
                .clearColor = meno::Color::fromRgb(0x182331)}) {}

private:
    meno::GameObject* floor_ = nullptr; // 소유자는 Scene이다.
    meno::GameObject* crate_ = nullptr;
    float direction_ = 1.f;
    float verticalSpeed_ = 0.f;
    bool grounded_ = true;
    double pause_ = jumpPause;

    void onStart() override {
        auto& scene = scenes().createScene();
        floor_ = &scene.createGameObject();
        floor_->transform().pos = {floorX, floorY};
        floor_->transform().magnitude = scale;
        floor_->sprite().setTexture(makeTexture(true));
        floor_->addComponent<meno::BoxCollider>(meno::Vec2f{},
            meno::Vec2f{static_cast<float>(floorWidthPixels), static_cast<float>(floorHeightPixels)});

        crate_ = &scene.createGameObject();
        crate_->transform().pos = {floorX + 32.f, floorY - crateSize};
        crate_->transform().magnitude = scale;
        crate_->sprite().setTexture(makeTexture(false));
        // Collider도 텍스처와 같은 로컬 크기. Transform 배율은 intersects가 적용한다.
        crate_->addComponent<meno::BoxCollider>(meno::Vec2f{},
            meno::Vec2f{static_cast<float>(cratePixels), static_cast<float>(cratePixels)});

        scene.setUpdateHandler([this](double dt) { moveBox(dt); });
        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("Failed to activate demo scene");
        }
    }

    void moveBox(double dt) {
        auto& position = crate_->transform().pos;
        const float step = static_cast<float>(dt);
        position.x += direction_ * speed * step;
        const float right = floorX + floorWidth - crateSize;
        if (position.x >= right) {
            position.x = right;
            direction_ = -1.f;
        } else if (position.x <= floorX) {
            position.x = floorX;
            direction_ = 1.f;
        }

        if (grounded_) {
            pause_ -= dt;
            if (pause_ > 0.0) return;
            grounded_ = false;
            verticalSpeed_ = -jumpSpeed; // 화면에서는 위쪽이 음수다.
        }

        const float previousBottom = position.y + crateSize;
        verticalSpeed_ += gravity * step;
        position.y += verticalSpeed_ * step;
        // intersects는 겹침만 검사한다. 위치 보정과 속도 변경은 게임에서 처리한다.
        if (verticalSpeed_ > 0.f && previousBottom <= floorY &&
            meno::intersects(*crate_, *floor_)) {
            position.y = floorY - crateSize;
            verticalSpeed_ = 0.f;
            grounded_ = true;
            pause_ = jumpPause;
        }
    }

};

} // namespace

int main() {
    try {
        JumpingBoxDemo demo;
        demo.run();
    } catch (const std::exception& error) {
        std::cerr << "Jumping box: " << error.what() << '\n';
        return 1;
    }
}
