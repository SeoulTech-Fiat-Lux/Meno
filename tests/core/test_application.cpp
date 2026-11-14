#include <meno/core/Application.hpp>
#include <meno/core/Clock.hpp>
#include <meno/core/Time.hpp>

#include "check.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

bool near(double left, double right) { return std::abs(left - right) < 1e-9; }

class TestApplication final : public meno::Application {
public:
    using Application::Application;

    std::vector<double> updateDeltas;
    std::vector<double> observedDeltas;
    std::vector<double> observedElapsedTimes;
    std::size_t renderCount = 0;
    std::vector<double> interpolationAlphas;

private:
    void update(double deltaTime) override {
        updateDeltas.push_back(deltaTime);
        observedDeltas.push_back(meno::Time::deltaTime());
        observedElapsedTimes.push_back(meno::Time::elapsedTime());
    }

    void render(double interpolationAlpha) override {
        ++renderCount;
        interpolationAlphas.push_back(interpolationAlpha);
        if (renderCount == 3) {
            stop();
        }
    }
};

class ThrowingApplication final : public meno::Application {
public:
    using Application::Application;

    std::size_t onStopCount = 0;
    std::size_t renderCount = 0;

private:
    void update(double) override {
        if (throwOnNextUpdate_) {
            throwOnNextUpdate_ = false;
            throw std::runtime_error("update failed");
        }
    }

    void render(double) override {
        ++renderCount;
        stop();
    }

    void onStop() override { ++onStopCount; }

    bool throwOnNextUpdate_ = true;
};

class ReentrantApplication final : public meno::Application {
public:
    bool rejectedReentry = false;

private:
    void onStart() override {
        try {
            run();
        } catch (const std::logic_error&) {
            rejectedReentry = true;
        }
        stop();
    }
};

class CallbackApplication final : public meno::Application {
public:
    using Application::Application;
    std::function<void()> startHandler;
    std::function<void()> eventsHandler;
    std::function<void()> updateHandler;
    std::function<void()> stopHandler;

private:
    void onStart() override {
        if (startHandler)
            startHandler();
    }
    void processEvents() override {
        if (eventsHandler)
            eventsHandler();
    }
    void update(double) override {
        if (updateHandler)
            updateHandler();
    }
    void onStop() override {
        if (stopHandler)
            stopHandler();
    }
};

void testExclusiveRun() {
    using namespace std::chrono_literals;
    meno::Clock::TimePoint now{};
    meno::Clock clock([&] { return now; }, [](meno::Clock::Duration) {});
    CallbackApplication outer({.fixedTimeStep = 0.01});
    CallbackApplication inner({.fixedTimeStep = 0.02});
    int innerStarts = 0;
    inner.startHandler = [&] {
        ++innerStarts;
        inner.stop();
    };
    int rejected = 0;
    const auto rejectNestedRun = [&] {
        const auto elapsed = meno::Time::elapsedTime();
        const auto fixedDelta = meno::Time::fixedDeltaTime();
        for (bool injectedClock : {false, true}) {
            try {
                if (injectedClock)
                    inner.run(clock);
                else
                    inner.run();
                MENO_CHECK(false);
            } catch (const std::logic_error&) {
                ++rejected;
            }
            MENO_CHECK(!inner.isRunning());
            MENO_CHECK(innerStarts == 0);
            MENO_CHECK(meno::Time::elapsedTime() == elapsed);
            MENO_CHECK(meno::Time::fixedDeltaTime() == fixedDelta);
        }
    };
    outer.startHandler = [&] { rejectNestedRun(); };
    // onStart() 시간은 첫 프레임에 들어가지 않으므로 프레임마다 시간을 흘린다.
    outer.eventsHandler = [&] { now += 10ms; };
    outer.updateHandler = [&] {
        rejectNestedRun();
        MENO_CHECK(near(meno::Time::elapsedTime(), 0.01));
        outer.stop();
    };
    outer.stopHandler = [&] {
        MENO_CHECK(!outer.isRunning());
        rejectNestedRun();
    };
    outer.run(clock);
    MENO_CHECK(rejected == 6);
    inner.run(clock);
    MENO_CHECK(innerStarts == 1);

    // onStart(), update(), onStop() 중 어디에서 예외를 던져도 전역 실행 플래그가 해제되어야 한다.
    for (int phase = 0; phase < 3; ++phase) {
        CallbackApplication failing({.fixedTimeStep = 0.01});
        failing.startHandler = [&] {
            if (phase == 0)
                throw std::runtime_error("start failed");
        };
        failing.eventsHandler = [&] { now += 10ms; };
        failing.updateHandler = [&] {
            if (phase == 1)
                throw std::runtime_error("update failed");
            failing.stop();
        };
        failing.stopHandler = [] { throw std::runtime_error("stop failed"); };
        try {
            failing.run(clock);
            MENO_CHECK(false);
        } catch (const std::runtime_error&) {
        }
        MENO_CHECK(!failing.isRunning());
        inner.run(clock);
        MENO_CHECK(innerStarts == phase + 2);
    }
}

// onStart()가 오래 걸려도 첫 프레임에 update()가 몰리지 않아야 한다.
void testSlowStartIsExcluded() {
    using namespace std::chrono_literals;
    meno::Clock::TimePoint now{};
    meno::Clock clock([&] { return now; }, [](meno::Clock::Duration) {});
    CallbackApplication app({.fixedTimeStep = 0.01, .maxUpdatesPerFrame = 8});
    int updates = 0;
    int frames = 0;
    app.startHandler = [&] { now += 1s; }; // 느린 로딩
    app.eventsHandler = [&] {
        ++frames;
        if (frames == 1)
            MENO_CHECK(updates == 0);
        if (frames == 2) {
            MENO_CHECK(updates == 0); // 첫 프레임에는 시간이 흐르지 않았다
            app.stop();
        }
        now += 10ms;
    };
    app.updateHandler = [&] { ++updates; };
    app.run(clock);
    MENO_CHECK(frames == 2);
    MENO_CHECK(meno::Time::realElapsedTime() < 0.02); // 로딩 1초가 빠졌다
}

// 0 이하뿐 아니라 NaN과 무한대도 거부해야 한다. 통과시키면 update()가 한 번도 호출되지 않는다.
void testRejectsInvalidTiming() {
    const auto rejects = [](meno::ApplicationConfig config) {
        try {
            CallbackApplication app(config);
            return false;
        } catch (const std::invalid_argument&) {
            return true;
        }
    };
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    MENO_CHECK(rejects({.fixedTimeStep = 0.0}));
    MENO_CHECK(rejects({.fixedTimeStep = -0.01}));
    MENO_CHECK(rejects({.fixedTimeStep = nan}));
    MENO_CHECK(rejects({.fixedTimeStep = inf}));
    MENO_CHECK(rejects({.maxUpdatesPerFrame = 0}));
    MENO_CHECK(!rejects({.fixedTimeStep = 0.01}));
}

// 상한에 걸린 프레임에서는 반올림 오차로 남은 시간이 fixedTimeStep 이상일 수 있다.
// 16ms 프레임 25번 뒤 1초 프레임이 오면 보정 전에는 alpha가 1.0000000000000013이 된다.
class AlphaApplication final : public meno::Application {
public:
    explicit AlphaApplication(meno::Clock::TimePoint& now)
        : Application({.fixedTimeStep = 0.1, .maxUpdatesPerFrame = 8}), now_(now) {}
    std::vector<double> alphas;

private:
    void render(double interpolationAlpha) override {
        using namespace std::chrono_literals;
        alphas.push_back(interpolationAlpha);
        if (alphas.size() <= 25)
            now_ += 16ms;
        else if (alphas.size() == 26)
            now_ += 1s;
        else
            stop();
    }
    meno::Clock::TimePoint& now_;
};

void testInterpolationAlphaStaysBelowOne() {
    meno::Clock::TimePoint now{};
    meno::Clock clock([&] { return now; }, [](meno::Clock::Duration) {});
    AlphaApplication app(now);
    app.run(clock);
    MENO_CHECK(app.alphas.size() == 27);
    bool inRange = true;
    for (const double alpha : app.alphas)
        inRange = inRange && alpha >= 0.0 && alpha < 1.0;
    MENO_CHECK(inRange);
}

} // namespace

int main() {
    using namespace std::chrono_literals;
    using TimePoint = meno::Clock::TimePoint;

    // 생성 시와 onStart() 직후는 0초이고, 이후 세 프레임에 5ms, 12ms, 100ms가 흐른다.
    const std::vector<TimePoint> times{TimePoint{}, TimePoint{}, TimePoint{} + 5ms,
                                       TimePoint{} + 17ms, TimePoint{} + 117ms};
    std::size_t nextTime = 0;
    meno::Clock clock([&] { return times.at(nextTime++); }, [](meno::Clock::Duration) {});

    TestApplication app({.fixedTimeStep = 0.01, .maxUpdatesPerFrame = 3});
    app.run(clock);

    MENO_CHECK(app.renderCount == 3);
    MENO_CHECK(app.interpolationAlphas.size() == 3);
    MENO_CHECK(near(app.interpolationAlphas[0], 0.5));
    MENO_CHECK(near(app.interpolationAlphas[1], 0.7));
    MENO_CHECK(app.interpolationAlphas[2] >= 0.0 && app.interpolationAlphas[2] < 1.0);
    // 첫 두 프레임에서 1회, 밀린 세 번째 프레임에서 상한인 3회.
    MENO_CHECK(app.updateDeltas.size() == 4);
    for (std::size_t index = 0; index < app.updateDeltas.size(); ++index) {
        MENO_CHECK(near(app.updateDeltas[index], 0.01));
        MENO_CHECK(near(app.observedDeltas[index], 0.01));
        MENO_CHECK(near(app.observedElapsedTimes[index], 0.01 * static_cast<double>(index + 1)));
    }
    MENO_CHECK(near(meno::Time::deltaTime(), 0.01));
    MENO_CHECK(near(meno::Time::fixedDeltaTime(), 0.01));
    MENO_CHECK(near(meno::Time::elapsedTime(), 0.04));
    MENO_CHECK(near(meno::Time::frameDeltaTime(), 0.03));
    MENO_CHECK(near(meno::Time::realElapsedTime(), 0.117));
    MENO_CHECK(!app.isRunning());

    const std::vector<TimePoint> retryTimes{TimePoint{}, TimePoint{}, TimePoint{} + 20ms,
                                            TimePoint{} + 20ms, TimePoint{} + 40ms};
    std::size_t nextRetryTime = 0;
    meno::Clock retryClock([&] { return retryTimes.at(nextRetryTime++); },
                           [](meno::Clock::Duration) {});
    ThrowingApplication throwingApp({.fixedTimeStep = 0.01});

    bool updateExceptionCaught = false;
    try {
        throwingApp.run(retryClock);
    } catch (const std::runtime_error&) {
        updateExceptionCaught = true;
    }

    MENO_CHECK(updateExceptionCaught);
    MENO_CHECK(!throwingApp.isRunning());
    MENO_CHECK(throwingApp.onStopCount == 0);

    throwingApp.run(retryClock);
    MENO_CHECK(!throwingApp.isRunning());
    MENO_CHECK(throwingApp.renderCount == 1);
    MENO_CHECK(throwingApp.onStopCount == 1);

    ReentrantApplication reentrantApp;
    reentrantApp.run();
    MENO_CHECK(reentrantApp.rejectedReentry);
    MENO_CHECK(!reentrantApp.isRunning());

    testExclusiveRun();
    testSlowStartIsExcluded();
    testRejectsInvalidTiming();
    testInterpolationAlphaStaysBelowOne();

    TimePoint limitedNow{};
    std::size_t waitCount = 0;
    meno::Clock limitedClock([&] { return limitedNow; },
                             [&](meno::Clock::Duration duration) {
                                 ++waitCount;
                                 limitedNow += duration;
                             });
    limitedClock.setFramerateLimit(60);

    const double expectedFrameTime = 1.0 / 60.0;
    const double clockTolerance = std::chrono::duration<double>(meno::Clock::Duration{1}).count();
    const double limitedDelta = limitedClock.restart();

    MENO_CHECK(std::abs(limitedDelta - expectedFrameTime) <= clockTolerance);
    MENO_CHECK(waitCount == 1);
    MENO_CHECK(std::abs(limitedClock.elapsed() - expectedFrameTime) <= clockTolerance);

    limitedNow += 20ms;
    MENO_CHECK(near(limitedClock.restart(), 0.02));
    MENO_CHECK(waitCount == 1);

    // resetLap()은 프레임 제한이 있어도 기다리지 않고, 다음 restart()의 기준만 옮긴다.
    limitedNow += 3ms;
    limitedClock.resetLap();
    MENO_CHECK(waitCount == 1);
    limitedNow += 20ms;
    MENO_CHECK(near(limitedClock.restart(), 0.02));
    MENO_CHECK(waitCount == 1);

    limitedClock.setFramerateLimit(0);
    limitedNow += 5ms;
    MENO_CHECK(near(limitedClock.restart(), 0.005));
    MENO_CHECK(waitCount == 1);

    return meno::test::report();
}
