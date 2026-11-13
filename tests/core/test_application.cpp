#include <meno/core/Application.hpp>
#include <meno/core/Clock.hpp>
#include <meno/core/Time.hpp>

#include "check.hpp"

#include <chrono>
#include <cmath>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>

namespace {

bool near(double left, double right) {
    return std::abs(left - right) < 1e-9;
}

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
    std::function<void()> updateHandler;
    std::function<void()> stopHandler;

private:
    void onStart() override { if (startHandler) startHandler(); }
    void update(double) override { if (updateHandler) updateHandler(); }
    void onStop() override { if (stopHandler) stopHandler(); }
};

void testExclusiveRun() {
    using namespace std::chrono_literals;
    meno::Clock::TimePoint now{};
    meno::Clock clock([&] { return now; }, [](meno::Clock::Duration) {});
    CallbackApplication outer({.fixedTimeStep = 0.01});
    CallbackApplication inner({.fixedTimeStep = 0.02});
    int innerStarts = 0;
    inner.startHandler = [&] { ++innerStarts; inner.stop(); };
    int rejected = 0;
    const auto rejectNestedRun = [&] {
        const auto elapsed = meno::Time::elapsedTime();
        const auto fixedDelta = meno::Time::fixedDeltaTime();
        for (bool injectedClock : {false, true}) {
            try {
                if (injectedClock) inner.run(clock);
                else inner.run();
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
    outer.startHandler = [&] { rejectNestedRun(); now += 10ms; };
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

    // 시작/업데이트/종료 콜백의 예외 모두 전역 실행 플래그를 해제해야 한다.
    for (int phase = 0; phase < 3; ++phase) {
        CallbackApplication failing({.fixedTimeStep = 0.01});
        failing.startHandler = [&] {
            if (phase == 0) throw std::runtime_error("start failed");
            now += 10ms;
        };
        failing.updateHandler = [&] {
            if (phase == 1) throw std::runtime_error("update failed");
            failing.stop();
        };
        failing.stopHandler = [] { throw std::runtime_error("stop failed"); };
        try {
            failing.run(clock);
            MENO_CHECK(false);
        } catch (const std::runtime_error&) {}
        MENO_CHECK(!failing.isRunning());
        inner.run(clock);
        MENO_CHECK(innerStarts == phase + 2);
    }
}

} // namespace

int main() {
    using namespace std::chrono_literals;
    using TimePoint = meno::Clock::TimePoint;

    // 생성 시 0초, 이후 프레임은 5ms, 12ms, 100ms가 흐른다.
    const std::vector<TimePoint> times{TimePoint{}, TimePoint{} + 5ms,
                                       TimePoint{} + 17ms, TimePoint{} + 117ms};
    std::size_t nextTime = 0;
    meno::Clock clock([&] { return times.at(nextTime++); },
                      [](meno::Clock::Duration) {});

    TestApplication app({.fixedTimeStep = 0.01,
                         .maxUpdatesPerFrame = 3});
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
        MENO_CHECK(near(app.observedElapsedTimes[index],
                    0.01 * static_cast<double>(index + 1)));
    }
    MENO_CHECK(near(meno::Time::deltaTime(), 0.01));
    MENO_CHECK(near(meno::Time::fixedDeltaTime(), 0.01));
    MENO_CHECK(near(meno::Time::elapsedTime(), 0.04));
    MENO_CHECK(near(meno::Time::frameDeltaTime(), 0.03));
    MENO_CHECK(near(meno::Time::realElapsedTime(), 0.117));
    MENO_CHECK(!app.isRunning());

    const std::vector<TimePoint> retryTimes{TimePoint{}, TimePoint{} + 20ms,
                                            TimePoint{} + 40ms};
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

    TimePoint limitedNow{};
    std::size_t waitCount = 0;
    meno::Clock limitedClock(
        [&] { return limitedNow; },
        [&](meno::Clock::Duration duration) {
            ++waitCount;
            limitedNow += duration;
        });
    limitedClock.setFramerateLimit(60);

    const double expectedFrameTime = 1.0 / 60.0;
    const double clockTolerance =
        std::chrono::duration<double>(meno::Clock::Duration{1}).count();
    const double limitedDelta = limitedClock.restart();

    MENO_CHECK(std::abs(limitedDelta - expectedFrameTime) <= clockTolerance);
    MENO_CHECK(waitCount == 1);
    MENO_CHECK(std::abs(limitedClock.elapsed() - expectedFrameTime) <= clockTolerance);

    limitedNow += 20ms;
    MENO_CHECK(near(limitedClock.restart(), 0.02));
    MENO_CHECK(waitCount == 1);

    limitedClock.setFramerateLimit(0);
    limitedNow += 5ms;
    MENO_CHECK(near(limitedClock.restart(), 0.005));
    MENO_CHECK(waitCount == 1);

    return meno::test::report();
}
