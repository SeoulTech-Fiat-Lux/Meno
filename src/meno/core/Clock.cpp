#include <meno/core/Clock.hpp>

#include <thread>
#include <utility>

namespace meno {

Clock::Clock()
    : Clock([] { return NativeClock::now(); },
            [](Duration duration) { std::this_thread::sleep_for(duration); }) {}

Clock::Clock(NowFunction now, WaitFunction wait)
    : now_(std::move(now)), wait_(std::move(wait)) {
    startTime_ = now_();
    lastTime_ = startTime_;
}

double Clock::restart() {
    TimePoint current = now_();
    const Duration elapsed = current - lastTime_;
    if (elapsed < minimumFrameDuration_) {
        wait_(minimumFrameDuration_ - elapsed);
        current = now_();
    }
    const std::chrono::duration<double> delta = current - lastTime_;
    lastTime_ = current;
    return delta.count();
}

void Clock::setFramerateLimit(unsigned int framesPerSecond) noexcept {
    if (framesPerSecond == 0) {
        minimumFrameDuration_ = Duration::zero();
        return;
    }
    minimumFrameDuration_ = std::chrono::duration_cast<Duration>(
        std::chrono::duration<double>(1.0 / static_cast<double>(framesPerSecond)));
}

double Clock::elapsed() const {
    return std::chrono::duration<double>(now_() - startTime_).count();
}

} // namespace meno
