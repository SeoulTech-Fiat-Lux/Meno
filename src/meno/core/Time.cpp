#include <meno/core/Time.hpp>

namespace meno {

double Time::deltaTime_ = 0.0;
double Time::fixedDeltaTime_ = 1.0 / 60.0;
double Time::elapsedTime_ = 0.0;
double Time::frameDeltaTime_ = 0.0;
double Time::realElapsedTime_ = 0.0;

double Time::deltaTime() noexcept { return deltaTime_; }
double Time::fixedDeltaTime() noexcept { return fixedDeltaTime_; }
double Time::elapsedTime() noexcept { return elapsedTime_; }
double Time::frameDeltaTime() noexcept { return frameDeltaTime_; }
double Time::realElapsedTime() noexcept { return realElapsedTime_; }

void Time::beginRun(double fixedDeltaTime) noexcept {
    deltaTime_ = 0.0;
    fixedDeltaTime_ = fixedDeltaTime;
    elapsedTime_ = 0.0;
    frameDeltaTime_ = 0.0;
    realElapsedTime_ = 0.0;
}

void Time::beginFrame(double frameDeltaTime, double realDeltaTime) noexcept {
    frameDeltaTime_ = frameDeltaTime;
    realElapsedTime_ += realDeltaTime;
}

void Time::advanceStep(double deltaTime) noexcept {
    deltaTime_ = deltaTime;
    elapsedTime_ += deltaTime;
}

} // namespace meno
