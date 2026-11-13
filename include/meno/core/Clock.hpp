#ifndef MENO_CLOCK_HPP
#define MENO_CLOCK_HPP

#include <chrono>
#include <functional>

namespace meno {

/// 실제 시간을 재는 시계입니다. 프레임 사이 시간을 재고, 프레임 제한이 있으면 그만큼 기다립니다.
///
/// 보통은 Application::run()이 만들어 쓰므로 직접 만들 일이 없습니다. 테스트에서는 NowFunction과
/// WaitFunction을 주입한 가짜 시계를 Application::run(Clock&)에 넘기면 실제로 기다리지 않습니다.
///
/// \code{.cpp}
/// meno::Clock::TimePoint fakeNow{};
/// meno::Clock clock([&] { return fakeNow; }, [&](meno::Clock::Duration d) { fakeNow += d; });
/// fakeNow += std::chrono::milliseconds(16);
/// const double seconds = clock.restart(); // 0.016
/// \endcode
class Clock {
public:
    /// 시간 계산에 쓰는 타입입니다. std::chrono::steady_clock 기준입니다.
    /// \{
    using NativeClock = std::chrono::steady_clock;
    using TimePoint = NativeClock::time_point;
    using Duration = NativeClock::duration;
    /// \}

    /// 현재 시각을 돌려주는 함수입니다.
    using NowFunction = std::function<TimePoint()>;
    /// 주어진 시간만큼 기다리는 함수입니다. 프레임 제한을 지킬 때 불립니다.
    using WaitFunction = std::function<void(Duration)>;

    /// 실제 시각과 실제 대기를 쓰는 시계를 만듭니다. 만든 순간부터 잽니다.
    Clock();
    /// 시각 함수와 대기 함수를 주입해 만듭니다. 만든 순간의 now() 결과부터 잽니다.
    Clock(NowFunction now, WaitFunction wait);

    /// 지난 restart() 이후(처음이면 만든 뒤) 흐른 시간을 초로 돌려주고, 기준을 지금으로 옮깁니다.
    ///
    /// 프레임 제한보다 빨리 불리면 남은 시간만큼 기다린 뒤 돌려줍니다.
    [[nodiscard]] double restart();
    /// 만든 뒤 흐른 시간(초)입니다. restart()와 상관없습니다.
    [[nodiscard]] double elapsed() const;
    // 프레임 제한은 Window가 아니라 시간 계층이 맡는다.
    /// 초당 최대 프레임 수를 정합니다. 0이면 제한하지 않습니다.
    ///
    /// Application::run()은 이 값을 ApplicationConfig::framerateLimit으로 덮어쓰므로, 보통은
    /// ApplicationConfig에서 정합니다.
    void setFramerateLimit(unsigned int framesPerSecond) noexcept;

private:
    NowFunction now_;
    WaitFunction wait_;
    TimePoint startTime_;
    TimePoint lastTime_;
    Duration minimumFrameDuration_{};
};

} // namespace meno

#endif // MENO_CLOCK_HPP
