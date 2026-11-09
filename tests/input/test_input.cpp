#include <meno/core/Application.hpp>
#include <meno/core/Clock.hpp>
#include <meno/input/Input.hpp>

#include "check.hpp"

#include <chrono>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using meno::Input;
using meno::Key;
using meno::MouseButton;
using meno::Vec2i;
namespace ev = meno::event;
using namespace std::chrono_literals;

// 창 없이 한 프레임을 흉내낸다. Window::pollEvents가 하는 일과 같다.
void frame(Input& input, std::initializer_list<meno::Event> events) {
    input.beginFrame();
    for (const meno::Event& event : events) {
        input.handleEvent(event);
    }
}

// 가상시계: 생성 시각은 0이고 이후 restart()마다 frameIntervals를 차례로 더해서 돌려준다.
// 준비한 간격보다 프레임이 더 돌 경우, at()이 예외를 던진다.
meno::Clock simulatedClock(std::vector<std::chrono::milliseconds> frameIntervals) {
    return meno::Clock([intervals = std::move(frameIntervals), next = std::size_t{0},
                        now = meno::Clock::TimePoint{}]()
                        mutable {
                            if (next > 0) {
                                now += intervals.at(next - 1);
                            }
                            ++next;
                            return now;
                        },
                        [](meno::Clock::Duration) {});
}

// --- update 밖: 틱 = 프레임 ---------------------------------------

void testKeyboard() {
    Input input;
    MENO_CHECK(!input.isDown(Key::A));
    MENO_CHECK(!input.wasPressed(Key::A));
    MENO_CHECK(!input.wasReleased(Key::A));

    frame(input, {ev::KeyPressed{Key::A}});
    MENO_CHECK(input.isDown(Key::A));
    MENO_CHECK(input.wasPressed(Key::A));
    MENO_CHECK(!input.wasReleased(Key::A));
    MENO_CHECK(!input.isDown(Key::B));

    // 다음 프레임: 키는 계속 눌려 있으나, "눌린 순간"은 지난 상태
    frame(input, {});
    MENO_CHECK(input.isDown(Key::A));
    MENO_CHECK(!input.wasPressed(Key::A));

    // OS의 자동 반복으로 같은 키의 KeyPressed을 전달받는 순간. 새로 누르지는 않은 상태.
    frame(input, {ev::KeyPressed{Key::A}});
    MENO_CHECK(input.isDown(Key::A));
    MENO_CHECK(!input.wasPressed(Key::A));

    frame(input, {ev::KeyReleased{Key::A}});
    MENO_CHECK(!input.isDown(Key::A));
    MENO_CHECK(input.wasReleased(Key::A));
    MENO_CHECK(!input.wasPressed(Key::A));

    frame(input, {});
    MENO_CHECK(!input.wasReleased(Key::A));

    // 한 프레임 안에서 눌렀다 떼도 KeyPressed와 KeyReleased를 모두 확인한다.
    frame(input, {ev::KeyPressed{Key::Space}, ev::KeyReleased{Key::Space}});
    MENO_CHECK(!input.isDown(Key::Space));
    MENO_CHECK(input.wasPressed(Key::Space));
    MENO_CHECK(input.wasReleased(Key::Space));

    // 누른 적 없는 키를 떼는 순간은 무시.
    // 이때, 창이 열리기 전부터 누르고 있던 키 역시 제외된다.
    frame(input, {ev::KeyReleased{Key::B}});
    MENO_CHECK(!input.wasReleased(Key::B));

    frame(input, {ev::KeyPressed{Key::Unknown}});
    MENO_CHECK(!input.isDown(Key::Unknown));
    MENO_CHECK(!input.wasPressed(Key::Unknown));

    // 범위 밖의 값은 무시하며 false를 반환한다.
    const auto invalid = static_cast<Key>(250);
    frame(input, {ev::KeyPressed{invalid}});
    MENO_CHECK(!input.isDown(invalid));
    MENO_CHECK(!input.wasPressed(invalid));
}

void testMouse() {
    Input input;
    frame(input, {ev::MouseButtonPressed{MouseButton::Left, {10, 20}}});
    MENO_CHECK(input.isDown(MouseButton::Left));
    MENO_CHECK(input.wasPressed(MouseButton::Left));
    MENO_CHECK(!input.isDown(MouseButton::Right));
    MENO_CHECK(input.mousePosition() == Vec2i(10, 20));

    frame(input, {ev::MouseMoved{{30, 40}}});
    MENO_CHECK(input.isDown(MouseButton::Left));
    MENO_CHECK(!input.wasPressed(MouseButton::Left));
    MENO_CHECK(input.mousePosition() == Vec2i(30, 40));

    frame(input, {ev::MouseButtonReleased{MouseButton::Left, {50, 60}}});
    MENO_CHECK(!input.isDown(MouseButton::Left));
    MENO_CHECK(input.wasReleased(MouseButton::Left));
    MENO_CHECK(input.mousePosition() == Vec2i(50, 60));

    // 휠의 값은 한 프레임 동안 이뤄진 것을 합산하며, 다음 프레임에는 초기화한다.
    frame(input, {ev::MouseWheelScrolled{1.f, {70, 80}}, ev::MouseWheelScrolled{-0.25f, {70, 80}}});
    MENO_CHECK(input.mouseWheelDelta() == 0.75f);
    MENO_CHECK(input.mousePosition() == Vec2i(70, 80));
    frame(input, {});
    MENO_CHECK(input.mouseWheelDelta() == 0.f);

    const auto invalid = static_cast<MouseButton>(200);
    frame(input, {ev::MouseButtonPressed{invalid, {0, 0}}});
    MENO_CHECK(!input.isDown(invalid));
}

void testText() {
    Input input;
    MENO_CHECK(input.text().empty());

    // 1·2·2·3·4바이트 UTF-8: a, é, Ж, 한, 😀
    frame(input, {ev::TextEntered{U'a'}, ev::TextEntered{U'\u00E9'}, ev::TextEntered{U'\u0416'},
                  ev::TextEntered{U'\uD55C'}, ev::TextEntered{U'\U0001F600'}});
    MENO_CHECK(input.text() == "a\xC3\xA9\xD0\x96\xED\x95\x9C\xF0\x9F\x98\x80");

    // 제어 문자(백스페이스, 엔터, Delete, C1)와 유니코드가 아닌 값은 제외한다.
    frame(input, {ev::TextEntered{U'\b'}, ev::TextEntered{U'\r'}, ev::TextEntered{U'\x7F'},
                  ev::TextEntered{U'\u0085'}, ev::TextEntered{char32_t{0xD800}},
                  ev::TextEntered{char32_t{0x110000}}, ev::TextEntered{U'b'}});
    MENO_CHECK(input.text() == "b");

    // 돌려받은 문자열은 복사본이라 다음 프레임에서도 유지된다.
    const std::string held = input.text();
    frame(input, {ev::TextEntered{U'z'}});
    MENO_CHECK(held == "b");
    MENO_CHECK(input.text() == "z");

    frame(input, {});
    MENO_CHECK(input.text().empty());
}

void testFocusLost() {
    Input input;
    frame(input, {ev::KeyPressed{Key::W}, ev::MouseButtonPressed{MouseButton::Right, {0, 0}}});

    // 포커스를 잃는 순간, 눌려 있던 모든 키를 뗀 것으로 처리한다.
    frame(input, {ev::FocusLost{}});
    MENO_CHECK(!input.isDown(Key::W));
    MENO_CHECK(input.wasReleased(Key::W));
    MENO_CHECK(!input.isDown(MouseButton::Right));
    MENO_CHECK(input.wasReleased(MouseButton::Right));

    // 누른 채로 Focus를 잃었다가 다시 돌아올 때, 동일한 키의 KeyPressed가 오면 새로 누른 것으로 간주한다.
    frame(input, {ev::KeyPressed{Key::W}});
    MENO_CHECK(input.wasPressed(Key::W));
}

void testActions() {
    Input input;
    input.bind("jump", Key::Space);
    input.bind("jump", Key::W);
    input.bind("jump", MouseButton::Left);

    MENO_CHECK(!input.isActionDown("jump"));
    MENO_CHECK(!input.isActionDown("unknown"));
    MENO_CHECK(!input.wasActionPressed("unknown"));

    frame(input, {ev::KeyPressed{Key::Space}});
    MENO_CHECK(input.isActionDown("jump"));
    MENO_CHECK(input.wasActionPressed("jump"));

    frame(input, {});
    MENO_CHECK(input.isActionDown("jump"));
    MENO_CHECK(!input.wasActionPressed("jump"));

    // Space를 누른 채 W를 더 눌러도 액션이 새로 눌린 것은 아니다.
    frame(input, {ev::KeyPressed{Key::W}});
    MENO_CHECK(!input.wasActionPressed("jump"));

    // 하나를 떼도 다른 하나가 눌려 있으면 액션은 계속 눌려 있다.
    frame(input, {ev::KeyReleased{Key::Space}});
    MENO_CHECK(input.isActionDown("jump"));
    MENO_CHECK(!input.wasActionReleased("jump"));

    frame(input, {ev::KeyReleased{Key::W}});
    MENO_CHECK(!input.isActionDown("jump"));
    MENO_CHECK(input.wasActionReleased("jump"));

    // 마우스 버튼도 같은 액션에 묶인다. 한 프레임 안에 눌렀다 떼도 놓치지 않는다.
    frame(input, {ev::MouseButtonPressed{MouseButton::Left, {}},
                  ev::MouseButtonReleased{MouseButton::Left, {}}});
    MENO_CHECK(!input.isActionDown("jump"));
    MENO_CHECK(input.wasActionPressed("jump"));
    MENO_CHECK(input.wasActionReleased("jump"));

    // 묶이지 않은 키는 액션에 영향이 없다. 이미 떼어진 액션이 다시 떼어지지도 않는다.
    frame(input, {ev::KeyPressed{Key::A}});
    MENO_CHECK(!input.isActionDown("jump"));
    MENO_CHECK(!input.wasActionReleased("jump"));

    // 이미 눌려 있는 키를 묶으면 눌린 상태로 시작하지만, 이번 틱에 눌린 것은 아니다.
    input.bind("fire", Key::A);
    MENO_CHECK(input.isActionDown("fire"));
    MENO_CHECK(!input.wasActionPressed("fire"));

    frame(input, {ev::FocusLost{}});
    MENO_CHECK(!input.isActionDown("fire"));
    MENO_CHECK(input.wasActionReleased("fire"));

    // 키 하나를 여러 액션이 같이 쓸 수 있다.
    input.bind("pause", Key::Escape);
    input.bind("back", Key::Escape);
    frame(input, {ev::KeyPressed{Key::Escape}});
    MENO_CHECK(input.wasActionPressed("pause"));
    MENO_CHECK(input.wasActionPressed("back"));

    input.unbind("jump");
    input.unbind("never-bound");
    frame(input, {ev::KeyPressed{Key::Space}});
    MENO_CHECK(!input.isActionDown("jump"));
    MENO_CHECK(!input.wasActionPressed("jump"));

    // 지운 뒤 다시 묶으면 지금 키 상태에서 새로 시작한다.
    input.bind("jump", Key::Space);
    MENO_CHECK(input.isActionDown("jump"));
    MENO_CHECK(!input.wasActionPressed("jump"));
}

// --- update 안: 틱 = fixed update---------------------------------

struct Observation {
    bool down = false;
    bool pressed = false;
    bool released = false;
    float wheel = 0.f;
    std::string text;
};

// 프레임마다 정해진 이벤트를 넣고, update와 render에서 본 Space의 상태를 기록한다.
class ScriptedApplication final : public meno::Application {
public:
    using Script = std::vector<std::vector<meno::Event>>;

    explicit ScriptedApplication(Script script, Script updateScript = {})
        : Application({.fixedTimeStep = 0.01}), script_(std::move(script)), updateScript_(std::move(updateScript)) {}

    Input input;
    std::vector<Observation> updates;
    std::vector<Observation> renders;

private:
    Observation observe() const {
        return {input.isDown(Key::Space), input.wasPressed(Key::Space),
                input.wasReleased(Key::Space), input.mouseWheelDelta(), input.text()};
    }

    void processEvents() override {
        input.beginFrame();
        for (const meno::Event& event : script_.at(frame_)) {
            input.handleEvent(event);
        }
    }

    void update(double) override {
        if (const std::size_t i = updates.size(); i < updateScript_.size()) {
            for (const meno::Event& event : updateScript_[i]) {
                input.handleEvent(event);
            }
        }
        updates.push_back(observe());
    }

    void render(double) override {
        renders.push_back(observe());
        if (++frame_ == script_.size()) {
                stop();
        }
    }

    Script script_, updateScript_;
    std::size_t frame_ = 0;
};

void testEventsBeforeUpdate() {
    auto clock = simulatedClock({4ms, 10ms, 30ms, 30ms, 10ms});

    ScriptedApplication app({
        {ev::KeyPressed{Key::Space}, ev::TextEntered{U'a'}, ev::MouseWheelScrolled{1.f, {}}},
        {ev::TextEntered{U'b'}, ev::MouseWheelScrolled{2.f, {}}},
        {},
        {ev::KeyReleased{Key::Space}, ev::TextEntered{U'c'}},
        {ev::KeyPressed{Key::Space}, ev::KeyReleased{Key::Space}},
    });
    app.run(clock);

    const auto& u = app.updates;
    const auto& r = app.renders;
    MENO_CHECK(u.size() == 8 && r.size() == 5);
    if (u.size() != 8 || r.size() != 5) {
        return;
    }

    // 프레임 0: update가 없다.
    // 그 사이 쌓인 키 및 글자 입력이나 휠 이동의 여부는 직후 발생하는 첫 update가 한 번에 처리한다.
    MENO_CHECK(u[0].down && u[0].pressed && !u[0].released);
    MENO_CHECK(u[0].text == "ab");
    MENO_CHECK(u[0].wheel == 3.f);

    // 프레임 2: 세 번의 update 중 어느 것도 이미 본 KeyPressed을 다시 보지 않는다.
    for (std::size_t i = 1; i <= 3; ++i) {
        MENO_CHECK(u[i].down && !u[i].pressed && !u[i].released);
        MENO_CHECK(u[i].text.empty());
        MENO_CHECK(u[i].wheel == 0.f);
    }

    // 프레임 3: 키를 뗐다. 세 번의 update 중 첫 번째만 이를 확인한다.
    // 새로 입력된 글자는 앞선 update가 본 "ab"와 섞이지 않는다.
    MENO_CHECK(u[4].text == "c" && u[5].text.empty());
    MENO_CHECK(!u[4].down && u[4].released);
    MENO_CHECK(!u[5].released);
    MENO_CHECK(!u[6].released);

    // 프레임 4: 한 프레임 안에 눌렀다 뗀 것도 update가 놓치지 않는다.
    MENO_CHECK(!u[7].down && u[7].pressed && u[7].released);

    // render: update 밖의 스코프이므로 프레임을 기준으로 판단한다.
    MENO_CHECK(r[0].pressed && r[0].text == "a" && r[0].wheel == 1.f);
    MENO_CHECK(!r[1].pressed && r[1].text == "b" && r[1].wheel == 2.f);
    MENO_CHECK(!r[2].pressed && !r[2].released);
    MENO_CHECK(r[3].released);
    MENO_CHECK(r[4].pressed && r[4].released);
}

void testEventsInUpdate() {
    auto clock = simulatedClock({0ms, 20ms, 30ms});

    ScriptedApplication app({
        {ev::TextEntered{U'a'}, {}},
        {ev::KeyPressed{Key::Space}, ev::TextEntered{U'b'}, ev::MouseWheelScrolled{1.f, {}}},
        {},
        {ev::KeyReleased{Key::Space}},
    });
    app.run(clock);

    const auto& u = app.updates;
    const auto& r = app.renders;
    MENO_CHECK(u.size() == 3 && r.size() == 5);
    if (u.size() != 3 || r.size() != 2) {
        return;
    }

    // update 안에서 넣은 입력은 같은 update에서 보인다.
    // update 밖에서 넣은 'a'와 같은 update로 묶인다.
    MENO_CHECK(u[0].down && u[0].pressed);
    MENO_CHECK(u[0].text == "ab" && u[0].wheel == 1.f);

    // 다음 update는 같은 입력을 다시 보지 않는다.
    MENO_CHECK(u[1].down && !u[1].pressed);
    MENO_CHECK(u[1].text.empty() && u[1].wheel == 0.f);

    // release도 마찬가지다.
    MENO_CHECK(!u[2].down && u[2].released);

    // render는 프레임을 기준으로 판정하므로 update 안에서 넣은 입력도 같은 프레임에서 보인다.
    MENO_CHECK(r[0].pressed && r[0].text == "ab" && r[0].wheel == 1.f);
    MENO_CHECK(r[1].released);
}

// update 한 번을 수행할 수 있는 프레임을 돈 뒤, 해당 update에서 A가 눌렸는지를 검사한다.
class PressCounter final : public meno::Application {
public:
    explicit PressCounter(const Input& input)
        : Application({.fixedTimeStep = 0.01}), input_(input) {}

    int presses = 0;
private:
    void update(double) override {
        if (input_.wasPressed(Key::A)) {
            ++presses;
        }
    }

    void render(double) override { stop(); }

    const Input& input_;
};

// 처음 생성할 때는 0ms, 그 뒤로는 15ms를 반환한다.
// fixedTimeStep이 10ms면 첫 프레임에 update가 한 번 작동한다.
meno::Clock oneUpdateClock() {
    using namespace std::chrono_literals;
    return meno::Clock(
        [calls = 0]() mutable { return meno::Clock::TimePoint{} + (calls++ == 0 ? 0ms : 15ms); },
        [](meno::Clock::Duration) {});
}

void testRunRestart() {
    //run 밖에서 들어온 KeyPressed는 다음 run의 첫 update가 확인한다.
    Input input;
    frame(input, {ev::KeyPressed{Key::A}});
    PressCounter first(input);
    auto firstClock = simulatedClock({15ms});
    first.run(firstClock);
    MENO_CHECK(first.presses == 1);

    // run을 다시 시작해도 update 번호를 0부터 다시 매기지 않는다.
    // 따라서 이전 run에서 본 KeyPressed를 다시 확인하지 않아도 된다.
    PressCounter second(input);
    auto secondClock = simulatedClock({15ms});
    second.run(secondClock);
    MENO_CHECK(second.presses == 0);
}

class ThrowingApplication final : public meno::Application {
private:
    void update(double) override { throw std::runtime_error("update failed!"); }
};

void testStepFlagClearedAfterException() {
    auto clock = simulatedClock({20ms});

    ThrowingApplication app;
    try {
        app.run(clock);
    } catch (const std::runtime_error&) {
    }

    // update가 예외로 끝날 때 "update 내부" 표시 역시 지운다.
    // 남아 있을 경우 아래 질의를 update를 기준으로 판정하기 때문에 false를 반환하는 오류가 생기기 때문이다.
    Input input;
    frame(input, {ev::KeyPressed{Key::A}});
    MENO_CHECK(input.wasPressed(Key::A));
}

} // namespace

int main() {
    testKeyboard();
    testMouse();
    testText();
    testFocusLost();
    testActions();
    testEventsBeforeUpdate();
    testEventsInUpdate();
    testRunRestart();
    testStepFlagClearedAfterException();
    return meno::test::report();
}