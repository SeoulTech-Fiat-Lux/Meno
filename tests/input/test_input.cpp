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

// 창 없이 한 프레임을 흉내낸다. Window::pollEvents가 하는 일과 같다.
void frame(Input& input, std::initializer_list<meno::Event> events) {
    input.beginFrame();
    for (const meno::Event& event : events) {
        input.handleEvent(event);
    }
}

// --- update 밖: 틱 = 프레임 ---------------------------------------

void testKeyboard() {
    Input input;
    MENO_CHECK(!input.isDown(Key::A));
    MENO_CHECK(!input.wasPressed(Key::A));
    MENO_CHECK(!input.wasReleased(Key::A));

    frame(input, {ev::KeyPressed{Key::A}});
    MENO_CHECK(!input.isDown(Key::A));
    MENO_CHECK(!input.wasPressed(Key::A));
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

    explicit ScriptedApplication(Script script)
        : Application({.fixedTimeStep = 0.01}), script_(std::move(script)) {}

    Input input;
    std::vector<Observation> updates;
    std::vector<Observation> renders;

private:
    Observation observe() const {
        return {input.isDown(Key::Space), input.wasPressed(Key::Space),
                input.wasPressed(Key::Space), input.mouseWheelDelta(), input.text()};
    }

    void processEvents() override {
        input.beginFrame();
        for (const meno::Event& event : script_.at(frame_)) {
            input.handleEvent(event);
        }
    }

    void update(double) override { updates.push_back(observe()); }

    void render(double) override {
        renders.push_back(observe());
        if (++frame_ == script_.size()) {
                stop();
        }
    }

    Script script_;
    std::size_t frame_ = 0;
};

void testFixedUpdate() {
    using namespace std::chrono_literals;
    using TimePoint = meno::Clock::TimePoint;

    // 프레임 간격 4, 10, 30, 30, 10ms라고 가정할 때,
    // fixedTimeStep이 10ms이므로 프레임마다 update는 각각 0, 1, 3, 3, 1번 동작한다.
    const std::vector<TimePoint> times{TimePoint{},        TimePoint{} + 4ms,  TimePoint{} + 14ms,
                                       TimePoint{} + 44ms, TimePoint{} + 74ms, TimePoint{} + 84ms};
    std::size_t nextTime = 0;
    meno::Clock clock([&] { return times.at(nextTime++); }, [](meno::Clock::Duration) {});

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
    MENO_CHECK(u.size() == 8);
    MENO_CHECK(r.size() == 5);
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
    meno::Clock firstClock = oneUpdateClock();
    first.run(firstClock);
    MENO_CHECK(first.presses == 1);

    // run을 다시 시작해도 update 번호를 0부터 다시 매기지 않는다.
    // 따라서 이전 run에서 본 KeyPressed를 다시 확인하지 않아도 된다.
    PressCounter second(input);
    meno::Clock secondClock = oneUpdateClock();
    second.run(secondClock);
    MENO_CHECK(second.presses == 0);
}

class ThrowingApplication final : public meno::Application {
private:
    void update(double) override { throw std::runtime_error("update failed!"); }
};

void testStepFlagClearedAfterException() {
    using namespace std::chrono_literals;
    using TimePoint = meno::Clock::TimePoint;

    const std::vector<TimePoint> times{TimePoint{}, TimePoint{} + 20ms};
    std::size_t nextTime = 0;
    meno::Clock clock([&] { return times.at(nextTime++); }, [](meno::Clock::Duration) {});

    ThrowingApplication app;
    try {
        app.run(clock);
    } catch (const std::runtime_error) {
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
    testFixedUpdate();
    testRunRestart();
    testStepFlagClearedAfterException();
    return meno::test::report();
}