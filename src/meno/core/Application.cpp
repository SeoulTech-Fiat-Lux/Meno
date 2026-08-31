#include <meno/core/Application.hpp>

#include <meno/core/Clock.hpp>
#include <meno/core/Time.hpp>

#include <algorithm>
#include <stdexcept>

namespace meno {

Application::Application(ApplicationConfig config) : config_(config) {
    if (config_.fixedTimeStep <= 0.0 || config_.maxUpdatesPerFrame == 0) {
        throw std::invalid_argument("Application timing values must be positive");
    }
}

void Application::run() {
    Clock clock;
    run(clock);
}

void Application::run(Clock& clock) {
    if (running_) {
        throw std::logic_error("Application is already running");
    }

    running_ = true;
    struct RunGuard {
        bool& running;

        ~RunGuard() noexcept { running = false; }
    } guard{running_};

    clock.setFramerateLimit(config_.framerateLimit);
    Time::beginRun(config_.fixedTimeStep);
    double accumulator = 0.0;
    onStart();

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
