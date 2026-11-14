// 바닥 위를 오가며 저절로 점프하는 상자 예제.
//
// 바닥과 상자를 각각 GameObject로 만들고 둘 다 BoxCollider를 붙인다. 상자는 입력 없이 바닥의 양
// 끝에서 방향을 바꾸고, 착지하면 잠깐 쉬었다가 다시 점프한다. 텍스처는 코드로 만들며, 창을 닫으면
// 종료한다.
//
// meno는 두 콜라이더가 겹치는지만 판정한다. 중력, 점프, 착지 처리는 모두 이 예제의 update
// 핸들러가 맡는 게임 로직이며, meno가 물리 엔진을 제공하지는 않는다.

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

// 움직임은 아래 네 값으로 조절한다.
constexpr float speed = 180.f;     // 가로 속도(픽셀/초)
constexpr float gravity = 1400.f;  // 중력 가속도(픽셀/초²)
constexpr float jumpSpeed = 620.f; // 점프할 때의 위쪽 속도(픽셀/초)
constexpr double jumpPause = 0.18; // 착지한 뒤 다음 점프까지 쉬는 시간(초)

meno::Texture makeTexture(bool floor) {
    const unsigned int width = floor ? floorWidthPixels : cratePixels;
    const unsigned int height = floor ? floorHeightPixels : cratePixels;
    std::vector<std::uint8_t> pixels(width * height * 4);
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            meno::Color color;
            if (floor) {
                const bool speckle = (x * 7 + y * 11) % 23 < 3;
                color = y < 3 ? meno::Color::fromRgb(0x79C88C)
                              : (speckle ? meno::Color::fromRgb(0x44566A)
                                         : meno::Color::fromRgb(0x334155));
            } else {
                const bool rim = x < 2 || y < 2 || x >= width - 2 || y >= height - 2;
                const bool brace = x == y || x + y == width - 1;
                color =
                    rim ? meno::Color::fromRgb(0xFFD18A)
                        : (brace ? meno::Color::fromRgb(0xEAAF62) : meno::Color::fromRgb(0xAC683D));
            }
            const auto index = (y * width + x) * 4;
            pixels[index] = color.r;
            pixels[index + 1] = color.g;
            pixels[index + 2] = color.b;
            pixels[index + 3] = 255;
        }
    }
    auto texture = meno::Texture::fromPixels({width, height}, pixels.data());
    if (!texture)
        throw std::runtime_error("Failed to create demo texture");
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
    // 두 GameObject는 Scene이 소유하므로 여기서는 포인터만 둔다.
    meno::GameObject* floor_ = nullptr;
    meno::GameObject* crate_ = nullptr;
    float direction_ = 1.f;
    float verticalSpeed_ = 0.f;
    bool grounded_ = true;
    double pause_ = jumpPause;

    void onStart() override {
        auto& scene = scenes().createScene();
        floor_ = &scene.createGameObject();
        floor_->transform().position = {floorX, floorY};
        floor_->transform().scale = scale;
        floor_->sprite().setTexture(makeTexture(true));
        floor_->addComponent<meno::BoxCollider>(meno::Vec2f{},
                                                meno::Vec2f{static_cast<float>(floorWidthPixels),
                                                            static_cast<float>(floorHeightPixels)});

        crate_ = &scene.createGameObject();
        crate_->transform().position = {floorX + 32.f, floorY - crateSize};
        crate_->transform().scale = scale;
        crate_->sprite().setTexture(makeTexture(false));
        // 콜라이더도 텍스처와 같은 로컬 크기다. Transform의 배율(4배)은 intersects()가 적용한다.
        crate_->addComponent<meno::BoxCollider>(
            meno::Vec2f{},
            meno::Vec2f{static_cast<float>(cratePixels), static_cast<float>(cratePixels)});

        scene.setUpdateHandler([this](double dt) { moveBox(dt); });
        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("Failed to activate demo scene");
        }
    }

    void moveBox(double dt) {
        auto& position = crate_->transform().position;
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
            if (pause_ > 0.0)
                return;
            grounded_ = false;
            verticalSpeed_ = -jumpSpeed; // y는 아래로 갈수록 커지므로 위쪽 속도는 음수다.
        }

        const float previousBottom = position.y + crateSize;
        verticalSpeed_ += gravity * step;
        position.y += verticalSpeed_ * step;
        // intersects()는 겹치는지만 판정하므로 위치 보정과 속도 변경은 게임이 직접 한다. 움직이기
        // 전에 바닥보다 위에 있었는지도 확인해, 옆에서 부딪힌 것을 착지로 보지 않는다.
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
