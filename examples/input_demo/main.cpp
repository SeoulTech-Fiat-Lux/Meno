// 키보드·마우스 입력 예제.
//
//   WASD / 방향키        공 이동 (액션 "left" "right" "up" "down")
//   Space / 마우스 오른쪽  대시 (액션 "dash". 한 번 누르면 한 번만 발동한다)
//   마우스 왼쪽           클릭한 곳으로 순간 이동
//   마우스 휠             공 크기
//   Enter                글자 입력 모드를 켜고 끈다. 켜져 있으면 타자로 글자를 쓰고
//                        Backspace로 지운다(이동은 멈춘다)
//   Esc                  종료
//
// 게임 로직은 고정 간격 update에서 입력을 읽는다. 한 프레임에 update가 몇 번 돌든
// wasPressed 계열은 한 번 누를 때 정확히 한 번만 true다. 화면 왼쪽 위의 대시 횟수로 확인할 수 있다.

#include <meno/core/Application.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/input/Input.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Vec2.hpp>
#include <meno/window/Window.hpp>

#include <algorithm>
#include <array>
#include <filesystem>
#include <optional>
#include <string>

namespace {

/// 한글 글리프가 있는 시스템 폰트를 찾아본다. 없으면 글자 없이 실행한다.
std::optional<meno::Font> findSystemFont() {
    const std::array<const char*, 4> candidates{
        "C:/Windows/Fonts/malgun.ttf",
        "/System/Library/Fonts/AppleSDGothicNeo.ttc",
        "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };
    for (const char* path : candidates) {
        if (std::filesystem::exists(path)) {
            if (auto font = meno::Font::loadFromFile(path)) {
                return font;
            }
        }
    }
    return std::nullopt;
}

/// UTF-8 문자열의 마지막 글자를 지운다. 한글은 3바이트이므로 바이트 하나만 지우면 깨진다.
void popLastCharacter(std::string& text) {
    while (!text.empty()) {
        const auto byte = static_cast<unsigned char>(text.back());
        text.pop_back();
        if ((byte & 0xC0) != 0x80) { // 10xxxxxx(이어지는 바이트)가 아니면 글자의 첫 바이트다.
            return;
        }
    }
}

class InputDemo final : public meno::Application {
public:
    InputDemo() : Application({.framerateLimit = 60}) {
        meno::Input& input = window_.input();
        input.bind("left", meno::Key::A);
        input.bind("left", meno::Key::Left);
        input.bind("right", meno::Key::D);
        input.bind("right", meno::Key::Right);
        input.bind("up", meno::Key::W);
        input.bind("up", meno::Key::Up);
        input.bind("down", meno::Key::S);
        input.bind("down", meno::Key::Down);
        input.bind("dash", meno::Key::Space);
        input.bind("dash", meno::MouseButton::Right);
    }

private:
    void processEvents() override {
        window_.pollEvents();
        // update 밖이므로 "이번 프레임에 눌렸는가"로 판정한다.
        if (!window_.isOpen() || window_.input().wasPressed(meno::Key::Escape)) {
            stop();
        }
    }

    void update(double deltaTime) override {
        const meno::Input& input = window_.input();
        const auto dt = static_cast<float>(deltaTime);
        previous_ = position_;

        // text()에는 이동 키로 친 글자까지 모두 들어온다. 글자를 받을 때와 조작할 때는
        // 게임이 나눠야 하므로, 여기서는 Enter로 입력 모드를 켜고 끈다.
        if (input.wasPressed(meno::Key::Enter)) {
            typing_ = !typing_;
        }
        if (typing_) {
            type(input);
        } else {
            move(input, dt);
        }
        dashTimeLeft_ = std::max(dashTimeLeft_ - dt, 0.f);

        if (input.wasPressed(meno::MouseButton::Left)) {
            position_ = static_cast<meno::Vec2f>(input.mousePosition());
            previous_ = position_; // 순간 이동이므로 보간하지 않는다.
        }
        radius_ = std::clamp(radius_ + input.mouseWheelDelta() * 4.f, 8.f, 120.f);
    }

    void move(const meno::Input& input, float dt) {
        meno::Vec2f direction;
        if (input.isActionDown("left")) {
            direction.x -= 1.f;
        }
        if (input.isActionDown("right")) {
            direction.x += 1.f;
        }
        if (input.isActionDown("up")) {
            direction.y -= 1.f;
        }
        if (input.isActionDown("down")) {
            direction.y += 1.f;
        }

        if (input.wasActionPressed("dash")) {
            ++dashCount_;
            dashTimeLeft_ = 0.15f;
        }
        const float speed = dashTimeLeft_ > 0.f ? 960.f : 240.f;
        if (direction.x != 0.f || direction.y != 0.f) {
            position_ += direction.normalized() * (speed * dt);
        }
    }

    void type(const meno::Input& input) {
        typed_ += input.text();
        if (input.wasPressed(meno::Key::Backspace)) {
            popLastCharacter(typed_);
        }
    }

    void render(double interpolationAlpha) override {
        const auto alpha = static_cast<float>(interpolationAlpha);
        const meno::Vec2f drawn = previous_ + (position_ - previous_) * alpha;
        const meno::Vec2f mouse = static_cast<meno::Vec2f>(window_.input().mousePosition());

        renderer_.beginFrame(meno::Color::fromRgb(0x1E2430));
        renderer_.drawCircle(
            drawn, radius_,
            {.fill = dashTimeLeft_ > 0.f ? meno::colors::Yellow : meno::Color::fromRgb(0x4C6EF5)});
        renderer_.drawCircle(mouse, 4.f, {.fill = meno::colors::White});

        if (font_) {
            const meno::TextParams hud{.characterSize = 20};
            renderer_.drawText(*font_, "dash: " + std::to_string(dashCount_), {16.f, 12.f}, hud);
            const std::string line = typing_ ? "> " + typed_ + "_" : "[Enter] " + typed_;
            renderer_.drawText(*font_, line, {16.f, 500.f}, hud);
        }
        renderer_.endFrame();
    }

    meno::Window window_{{960u, 540u}, "meno - 입력"};
    meno::Renderer renderer_{window_};
    std::optional<meno::Font> font_ = findSystemFont();

    meno::Vec2f position_{480.f, 270.f};
    meno::Vec2f previous_ = position_;
    float radius_ = 32.f;
    float dashTimeLeft_ = 0.f;
    int dashCount_ = 0;
    bool typing_ = false;
    std::string typed_;
};

} // namespace

int main() {
    InputDemo demo;
    demo.run();
}
