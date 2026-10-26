// 외부 에셋 없이 두 Scene을 3초마다 전환한다.
#include <meno/core/Application.hpp>
#include <meno/core/SceneManager.hpp>
#include <meno/core/Window.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {

constexpr double sceneDuration = 3.0;

class SceneTransitionDemo final : public meno::Application {
public:
    SceneTransitionDemo() : Application({.framerateLimit = 60}) {}

    ~SceneTransitionDemo() override {
        // run()에서 예외가 나더라도 콜백이 참조하는 멤버와 창보다 먼저 정리한다.
        (void)manager_.shutdown();
    }

private:
    meno::Window window_{{800u, 600u}, "meno - Scene transition (every 3 seconds)"};
    meno::Renderer renderer_{window_};
    meno::SceneManager& manager_ = meno::SceneManager::instance();
    double elapsed_ = 0.0;
    meno::Color accent_ = meno::colors::White;

    void configureScene(meno::Scene& scene, meno::Scene::SceneID next,
                        const char* name, meno::Color color, float direction,
                        const std::shared_ptr<const meno::Texture>& texture) {
        auto& object = scene.createGameObject();
        object.transform().pos = {400.f, 280.f};
        object.transform().magnitude = 100.f;
        object.sprite().origin = {0.5f, 0.5f};
        object.sprite().tint = color;
        object.sprite().setTexture(texture);

        scene.setEnterHandler([this, &object, name, color] {
            elapsed_ = 0.0;
            accent_ = color;
            object.transform().pos = {400.f, 280.f};
            object.transform().rotation = 0;
            std::cout << "[enter] " << name << std::endl;
        });
        scene.setUpdateHandler([this, &object, next, direction](double dt) {
            elapsed_ += dt;
            object.transform().pos.x =
                400.f + direction * 180.f * static_cast<float>(std::sin(elapsed_ * 2.0));
            object.transform().rotation = static_cast<int>(direction * elapsed_ * 90.0);

            if (elapsed_ >= sceneDuration) {
                // 콜백 실행 중에는 예약만 한다. update()가 콜백을 마친 뒤
                // 기존 Scene의 onExit, 다음 Scene의 onEnter를 호출한다.
                if (!manager_.changeScene(next)) {
                    throw std::runtime_error("Next scene does not exist");
                }
            }
        });
        scene.setExitHandler([name] {
            std::cout << "[exit]  " << name << std::endl;
        });
    }

    void onStart() override {
        constexpr std::array<std::uint8_t, 4> pixel{255, 255, 255, 255};
        auto texture = meno::Texture::fromPixels({1u, 1u}, pixel.data());
        if (!texture) throw std::runtime_error("Failed to create demo texture");
        auto sharedTexture = std::make_shared<meno::Texture>(std::move(*texture));

        auto& blue = manager_.createScene();
        auto& orange = manager_.createScene();
        configureScene(blue, orange.id(), "Blue", meno::Color::fromRgb(0x4DABF7),
                       1.f, sharedTexture);
        configureScene(orange, blue.id(), "Orange", meno::Color::fromRgb(0xFFA94D),
                       -1.f, sharedTexture);

        std::cout << "Blue <-> Orange every 3 seconds. Close the window to quit.\n";
        if (!manager_.changeScene(blue.id())) {
            throw std::runtime_error("Failed to activate initial scene");
        }
    }

    void processEvents() override {
        window_.pollEvents();
        if (!window_.isOpen()) stop();
    }

    void update(double dt) override { manager_.update(dt); }

    void render(double) override {
        renderer_.beginFrame(meno::Color::fromRgb(0x1E2430));
        manager_.draw(renderer_); // 활성 Scene의 객체만 그린다.
        renderer_.drawRect({{80.f, 500.f}, {640.f, 12.f}},
                           {.fill = meno::Color::fromRgb(0x394252)});
        const float progress = static_cast<float>(std::clamp(elapsed_ / sceneDuration, 0.0, 1.0));
        renderer_.drawRect({{80.f, 500.f}, {640.f * progress, 12.f}}, {.fill = accent_});
        renderer_.endFrame();
    }

    void onStop() override {
        // Scene/Sprite의 GPU 리소스를 Window가 파괴되기 전에 해제한다.
        (void)manager_.shutdown();
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
