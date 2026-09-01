#ifndef MENO_TIME_HPP
#define MENO_TIME_HPP

namespace meno {

class Application;

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

    static void beginRun(double fixedDeltaTime) noexcept;
    static void beginFrame(double frameDeltaTime, double realDeltaTime) noexcept;
    static void advanceStep(double deltaTime) noexcept;

    static double deltaTime_;
    static double fixedDeltaTime_;
    static double elapsedTime_;
    static double frameDeltaTime_;
    static double realElapsedTime_;
};

} // namespace meno

#endif  // MENO_TIME_HPP