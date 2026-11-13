#ifndef MENO_TIME_HPP
#define MENO_TIME_HPP

#include <cstdint>

namespace meno {

class Application;
namespace detail {
struct TimeAccess;
}

/// Application이 관리하는 게임 시간 정보.
class Time final {
public:
    /// 현재 fixed update의 시간 간격.
    [[nodiscard]] static double deltaTime() noexcept;
    /// Application에 설정된 고정 시간 간격.
    [[nodiscard]] static double fixedDeltaTime() noexcept;
    /// 처리 완료된 fixed update 기준의 누적 시뮬레이션 시간.
    [[nodiscard]] static double elapsedTime() noexcept;
    /// 현재 프레임에서 처리 가능한 fixed update 수로 제한된 시간 간격.
    [[nodiscard]] static double frameDeltaTime() noexcept;
    /// Clock이 측정한 실제 누적 시간.
    [[nodiscard]] static double realElapsedTime() noexcept;

private:
    friend class Application;
    /// 라이브러리 내부 Input이 update 단계 정보를 읽어들이는 통로.
    /// 정의는 src/meno/core/TimeAccess.hpp에 선언되어 있다.
    friend struct detail::TimeAccess;

    static void beginRun(double fixedDeltaTime) noexcept;
    static void beginFrame(double frameDeltaTime, double realDeltaTime) noexcept;
    /// update 직전에 호출한다. 단계 번호를 1 올리고서 해당 스텝이 update 안에 있다고 표시한다.
    static void advanceStep(double deltaTime) noexcept;
    /// update가 끝나거나 update 상태를 벗어날 때 호출한다.
    static void endStep() noexcept;

    static double deltaTime_;
    static double fixedDeltaTime_;
    static double elapsedTime_;
    static double frameDeltaTime_;
    static double realElapsedTime_;
    /// 현재까지 수행된 update 횟수. run이 다시 시작해도 초기화되지 않는다.
    static std::uint64_t stepNumber_;
    /// 현재 update 안에 있는지 여부.
    static bool inStep_;
};

} // namespace meno

#endif // MENO_TIME_HPP
