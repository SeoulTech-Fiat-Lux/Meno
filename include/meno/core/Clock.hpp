#ifndef MENO_CLOCK_HPP
#define MENO_CLOCK_HPP

#include <chrono>
#include <functional>

namespace meno {

/// 실제 시간을 측정하는 고정밀도 시계.
/// NowFunction과 WaitFunction을 주입하면 실제 대기 없이 단위 테스트할 수 있다.
class Clock {
public:
    using NativeClock = std::chrono::steady_clock;
    using TimePoint = NativeClock::time_point;
    using Duration = NativeClock::duration;
    using NowFunction = std::function<TimePoint()>;
    using WaitFunction = std::function<void(Duration)>;

    Clock();
    Clock(NowFunction now, WaitFunction wait);

    [[nodiscard]] double restart();
    [[nodiscard]] double elapsed() const;
    /// 0은 제한 없음. Window가 아닌 시간 계층이 프레임 제한을 담당한다.
    void setFramerateLimit(unsigned int framesPerSecond) noexcept;

private:
    NowFunction now_;
    WaitFunction wait_;
    TimePoint startTime_;
    TimePoint lastTime_;
    Duration minimumFrameDuration_{};
};

} // namespace meno

#endif  // MENO_CLOCK_HPP