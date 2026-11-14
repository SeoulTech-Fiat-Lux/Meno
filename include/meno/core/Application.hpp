#ifndef MENO_APPLICATION_HPP
#define MENO_APPLICATION_HPP

#include <cstddef>

namespace meno {

/// \addtogroup core
/// \{

class Clock;

/// Application 루프의 시간 설정입니다.
struct ApplicationConfig {
    /// fixed update 한 번의 시간 간격(초). 양의 유한수가 아니면(0 이하, NaN, 무한대)
    /// Application 생성자가 예외를 던집니다.
    double fixedTimeStep = 1.0 / 60.0;
    /// 한 프레임에 호출하는 update()의 최대 횟수. 0이면 Application 생성자가 예외를 던집니다.
    ///
    /// 프레임이 fixedTimeStep × maxUpdatesPerFrame보다 오래 걸리면 넘친 시간은 버립니다.
    /// 그래서 처리가 밀려도 게임이 멈추지 않고 느려집니다.
    std::size_t maxUpdatesPerFrame = 8;
    /// 초당 최대 프레임 수. 0이면 제한하지 않습니다.
    unsigned int framerateLimit = 0;
};

/// 고정 시간 간격(fixed timestep) 게임 루프의 기본 클래스입니다.
///
/// 창과 장면이 필요한 보통의 게임은 이 클래스를 상속한 Game을 씁니다. 루프만 필요할 때는
/// Application을 직접 상속합니다.
///
/// 한 프레임의 순서:
/// 1. processEvents()를 한 번 호출합니다.
/// 2. 흐른 시간만큼 update()를 0번 이상 호출합니다(최대 maxUpdatesPerFrame번).
/// 3. render()를 한 번 호출합니다.
///
/// 루프에 들어가기 전에 onStart()를, stop()으로 루프가 끝난 뒤에 onStop()을 호출합니다.
/// 한 번에 하나의 Application만 실행할 수 있습니다.
///
/// \code{.cpp}
/// class Demo : public meno::Application {
/// protected:
///     void update(double fixedDeltaTime) override {
///         // 게임 상태를 fixedDeltaTime(초)만큼 진행합니다.
///     }
///     void render(double interpolationAlpha) override {
///         // 그립니다.
///     }
/// };
///
/// Demo demo;
/// demo.run(); // stop()이 호출될 때까지 돌아오지 않습니다.
/// \endcode
///
/// \see Game, Time, ApplicationConfig
class Application {
public:
    /// 루프 설정으로 만듭니다.
    /// \throws std::invalid_argument fixedTimeStep이 양의 유한수가 아니거나
    /// maxUpdatesPerFrame이 0일 때
    explicit Application(ApplicationConfig config = {});
    virtual ~Application() = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /// 루프를 실행합니다. stop()으로 루프가 끝나면 onStop()을 호출한 뒤에 반환합니다.
    ///
    /// 시간은 실제 시계로 재며 ApplicationConfig::framerateLimit을 적용합니다. 콜백에서 던진
    /// 예외는 실행 상태를 정리한 뒤에 그대로 다시 던집니다. 이때 onStop()은 호출되지 않습니다.
    ///
    /// \throws std::logic_error 이미 실행 중인 Application이 있을 때(자기 자신 포함)
    virtual void run();
    /// run()과 같지만, 주입한 Clock으로 시간을 잽니다. 가짜 시계로 테스트할 때 씁니다.
    ///
    /// Clock에 미리 정한 프레임 제한은 ApplicationConfig::framerateLimit으로 덮어씁니다.
    ///
    /// \throws std::logic_error 이미 실행 중인 Application이 있을 때(자기 자신 포함)
    virtual void run(Clock& clock);
    /// 루프를 멈춥니다. 이후에는 update()와 render()가 더 호출되지 않고, onStop()이 호출된
    /// 뒤에 run()이 돌아옵니다. 실행 중이 아닐 때 호출하면 아무 일도 일어나지 않습니다.
    void stop() noexcept;
    /// 루프가 실행 중이면 true입니다. stop()을 호출하면 바로 false가 되므로 onStop() 안에서도
    /// false입니다.
    [[nodiscard]] bool isRunning() const noexcept;

protected:
    /// 루프에 들어가기 전에 한 번 호출됩니다. 에셋 로딩과 같은 준비를 여기서 합니다.
    ///
    /// 여기서 걸린 시간은 게임 시간과 Time::realElapsedTime()에 들어가지 않으므로, 로딩이 길어도
    /// 첫 프레임에 update()가 몰리지 않습니다.
    virtual void onStart();
    /// 프레임마다 update()보다 먼저 한 번 호출됩니다. 창 이벤트와 입력을 여기서 처리합니다.
    virtual void processEvents();
    /// 고정 시간 간격마다 호출됩니다. 게임 상태를 여기서 바꿉니다.
    ///
    /// fixedDeltaTime은 항상 ApplicationConfig::fixedTimeStep(초)입니다. 한 프레임에 0번 호출될
    /// 수도, 여러 번 호출될 수도 있습니다. Vec2f와 곱할 때는 float로 바꿔야 합니다.
    virtual void update(double fixedDeltaTime);
    /// 프레임마다 update()가 끝난 뒤에 한 번 호출됩니다. 그리기를 여기서 합니다.
    ///
    /// interpolationAlpha는 다음 update()까지 진행된 비율(0 이상 1 미만)입니다. 이전 상태와 현재
    /// 상태를 보간하여 움직임을 부드럽게 할 때 쓰며, 보간하지 않으면 무시해도 됩니다.
    virtual void render(double interpolationAlpha);
    /// stop()으로 루프가 끝난 뒤에 한 번 호출됩니다. 콜백이 예외로 끝나면 호출되지 않습니다.
    virtual void onStop();

private:
    ApplicationConfig config_;
    bool running_ = false;
};

/// \}

} // namespace meno

#endif // MENO_APPLICATION_HPP
