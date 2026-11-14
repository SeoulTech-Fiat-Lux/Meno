// 두 Scene을 3초마다 번갈아 전환하는 예제.
//
// Game을 상속하고 onStart()에서 파란 Scene과 주황 Scene을 만든다. 각 Scene에서는 그 색의 사각형이
// 좌우로 오가며 회전하고, 3초가 지나면 update 핸들러가 다음 Scene으로의 전환을 요청한다. enter와
// exit 핸들러는 호출될 때마다 콘솔에 Scene 이름을 출력한다. 에셋 파일 없이 실행되며, 창을 닫으면
// 종료한다.

#include <meno/app/Game.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>
#include <meno/scene/SceneManager.hpp>
#include <meno/window/Window.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

constexpr double sceneDuration = 3.0;

class SceneTransitionDemo final : public meno::Game {
public:
    SceneTransitionDemo()
        : Game({.application = {.framerateLimit = 60},
                .windowSize = {800u, 600u},
                .title = "meno - Scene transition (every 3 seconds)",
                .clearColor = meno::Color::fromRgb(0x1E2430)}) {}

private:
    double elapsed_ = 0.0;

    void configureScene(meno::Scene& scene, meno::Scene::SceneID next, const char* name,
                        meno::Color color, float direction,
                        const std::shared_ptr<const meno::Texture>& texture) {
        auto& object = scene.createGameObject();
        object.transform().position = {400.f, 280.f};
        object.transform().scale = 100.f;
        object.sprite().origin = {0.5f, 0.5f};
        object.sprite().tint = color;
        object.sprite().setTexture(texture);

        scene.setEnterHandler([this, &object, name, color] {
            elapsed_ = 0.0;
            object.transform().position = {400.f, 280.f};
            object.transform().rotation = 0;
            std::cout << "[enter] " << name << std::endl;
        });
        scene.setUpdateHandler([this, &object, next, direction](double dt) {
            elapsed_ += dt;
            object.transform().position.x =
                400.f + direction * 180.f * static_cast<float>(std::sin(elapsed_ * 2.0));
            object.transform().rotation = static_cast<float>(direction * elapsed_ * 90.0);

            if (elapsed_ >= sceneDuration) {
                // 핸들러 안에서는 전환이 예약만 된다. 이 update 핸들러가 끝나면 바로 SceneManager가
                // 이전 Scene의 exit 핸들러와 다음 Scene의 enter 핸들러를 차례로 호출한다.
                if (!scenes().changeScene(next)) {
                    throw std::runtime_error("Next scene does not exist");
                }
            }
        });
        scene.setExitHandler([name] { std::cout << "[exit]  " << name << std::endl; });
    }

    void onStart() override {
        constexpr std::array<std::uint8_t, 4> pixel{255, 255, 255, 255};
        auto texture = meno::Texture::fromPixels({1u, 1u}, pixel.data());
        if (!texture)
            throw std::runtime_error("Failed to create demo texture");
        auto sharedTexture = std::make_shared<meno::Texture>(std::move(*texture));

        auto& blue = scenes().createScene();
        auto& orange = scenes().createScene();
        configureScene(blue, orange.id(), "Blue", meno::Color::fromRgb(0x4DABF7), 1.f,
                       sharedTexture);
        configureScene(orange, blue.id(), "Orange", meno::Color::fromRgb(0xFFA94D), -1.f,
                       sharedTexture);

        std::cout << "Blue <-> Orange every 3 seconds. Close the window to quit.\n";
        if (!scenes().changeScene(blue.id())) {
            throw std::runtime_error("Failed to activate initial scene");
        }
    }
};

} // namespace

int main() {
    try {
        SceneTransitionDemo demo;
        demo.run();
    } catch (const std::exception& error) {
        std::cerr << "Scene transition demo: " << error.what() << '\n';
        return 1;
    }
}
