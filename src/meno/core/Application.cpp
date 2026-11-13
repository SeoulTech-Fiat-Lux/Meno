#include <meno/core/Application.hpp>

#include <meno/core/Clock.hpp>
#include <meno/core/Time.hpp>

#include <algorithm>
#include <stdexcept>

namespace meno {
namespace {

// Time은 전역 상태이므로 단일 스레드에서 Application 루프의 중첩 실행을 막는다.
bool applicationRunActive = false;

} // namespace

Application::Application(ApplicationConfig config) : config_(config) {
    if (config_.fixedTimeStep <= 0.0 || config_.maxUpdatesPerFrame == 0) {
        throw std::invalid_argument("Application timing values must be positive");
    }
}

void Application::run() {
    Clock clock;
    // 가상 dispatch는 공개 run() 진입 시 이미 끝났다. 같은 객체의 Clock 버전을
    // 직접 호출해 파생 클래스 wrapper가 중복 실행되지 않게 한다.
    Application::run(clock);
}

void Application::run(Clock& clock) {
    if (applicationRunActive) {
        throw std::logic_error("An Application is already running");
    }

    applicationRunActive = true;
    running_ = true;
    struct RunGuard {
        bool& running;

        explicit RunGuard(bool& flag) : running(flag) {}
        RunGuard(const RunGuard&) = delete;
        RunGuard& operator=(const RunGuard&) = delete;

        ~RunGuard() noexcept {
            running = false;
            Time::endStep();
            applicationRunActive = false;
        }
    } guard{running_};

    clock.setFramerateLimit(config_.framerateLimit);
    Time::beginRun(config_.fixedTimeStep);
    double accumulator = 0.0;
    onStart();
    // onStart()의 로딩 시간이 첫 프레임에 섞여 update()가 몰리지 않도록 기준점을 옮긴다.
    // restart()는 프레임 제한 때문에 기다릴 수 있으므로 쓰지 않는다.
    clock.resetLap();

    const double maxFrameTime =
        config_.fixedTimeStep * static_cast<double>(config_.maxUpdatesPerFrame);

    while (running_) {
        const double realFrameTime = std::max(clock.restart(), 0.0);
        const double frameTime = std::min(realFrameTime, maxFrameTime);
        Time::beginFrame(frameTime, realFrameTime);
        accumulator += frameTime;

        processEvents();

        std::size_t updateCount = 0;
        while (running_ && accumulator >= config_.fixedTimeStep &&
               updateCount < config_.maxUpdatesPerFrame) {
            Time::advanceStep(config_.fixedTimeStep);
            update(config_.fixedTimeStep);
            Time::endStep();
            accumulator -= config_.fixedTimeStep;
            ++updateCount;
        }

        if (running_) {
            render(accumulator / config_.fixedTimeStep);
        }
    }

    onStop();
}

void Application::stop() noexcept { running_ = false; }
bool Application::isRunning() const noexcept { return running_; }
void Application::onStart() {}
void Application::processEvents() {}
void Application::update(double) {}
void Application::render(double) {}
void Application::onStop() {}

} // namespace meno
