#ifndef MENO_TIME_HPP
#define MENO_TIME_HPP

#include <cstdint>

namespace meno {

class Application;
namespace detail {
struct TimeAccess;
}

// Time은 전역 상태이므로, 한 번에 하나의 Application만 실행하도록 Application이 막는다.
/// Application 루프가 관리하는 게임 시간입니다. 어디서든 정적 함수로 읽으며 단위는 초입니다.
///
/// 게임 시간(elapsedTime)은 fixed update가 진행된 만큼만 흐릅니다. 프레임이 밀려 시간을 버리면
/// 실제 시간(realElapsedTime)보다 느려집니다. Application::run()이 시작되면 fixedDeltaTime()을
/// 뺀 값은 0부터 다시 셉니다.
///
/// \code{.cpp}
/// // 3초가 지나면 안내 문구를 숨깁니다.
/// if (meno::Time::elapsedTime() > 3.0) {
///     showHint = false;
/// }
/// \endcode
///
/// \see Application, ApplicationConfig
class Time final {
public:
    /// 현재 fixed update의 시간 간격입니다. update() 안에서는 fixedDeltaTime()과 같고,
    /// run()이 시작된 뒤 첫 update() 전에는 0입니다.
    [[nodiscard]] static double deltaTime() noexcept;
    /// ApplicationConfig::fixedTimeStep 값입니다.
    [[nodiscard]] static double fixedDeltaTime() noexcept;
    /// 지금까지 진행된 fixed update 시간의 합입니다. update() 안에서는 처리 중인 단계까지
    /// 포함합니다.
    [[nodiscard]] static double elapsedTime() noexcept;
    /// 이번 프레임에 흐른 시간입니다. fixedTimeStep × maxUpdatesPerFrame을 넘으면 그 값으로
    /// 자릅니다.
    [[nodiscard]] static double frameDeltaTime() noexcept;
    /// run()이 시작된 뒤 실제로 흐른 시간의 합입니다. 자르지 않은 값이며 onStart()에서 걸린
    /// 시간도 들어갑니다.
    [[nodiscard]] static double realElapsedTime() noexcept;

private:
    friend class Application;
    // 라이브러리 내부 Input이 update 단계 정보를 읽어들이는 통로.
    // 정의는 src/meno/core/TimeAccess.hpp에 있다.
    friend struct detail::TimeAccess;

    static void beginRun(double fixedDeltaTime) noexcept;
    static void beginFrame(double frameDeltaTime, double realDeltaTime) noexcept;
    // update 직전에 호출한다. 단계 번호를 1 올리고서 해당 스텝이 update 안에 있다고 표시한다.
    static void advanceStep(double deltaTime) noexcept;
    // update가 끝나거나 update 상태를 벗어날 때 호출한다.
    static void endStep() noexcept;

    static double deltaTime_;
    static double fixedDeltaTime_;
    static double elapsedTime_;
    static double frameDeltaTime_;
    static double realElapsedTime_;
    // 현재까지 수행된 update 횟수. run이 다시 시작해도 초기화되지 않는다.
    static std::uint64_t stepNumber_;
    // 현재 update 안에 있는지 여부.
    static bool inStep_;
};

} // namespace meno

#endif // MENO_TIME_HPP
