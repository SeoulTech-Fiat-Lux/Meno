#include <meno/app/Game.hpp>
#include <meno/core/Clock.hpp>

#include <stdexcept>

namespace meno {
namespace {

struct RunInvocationGuard {
    bool& active;

    explicit RunInvocationGuard(bool& flag) : active(flag) { active = true; }
    ~RunInvocationGuard() { active = false; }
};

} // namespace

Game::Game(GameConfig config)
    : Application(config.application), window_(config.windowSize, config.title), renderer_(window_),
      clearColor_(config.clearColor) {}

Game::~Game() noexcept {
    // 멤버 소멸이 시작되기 전에 장면을 명시적으로 폐기한다. 컴포넌트 소멸자는
    // 아직 살아 있는 Window, Renderer, ResourceManager를 사용할 수 있다.
    // 이 시점에는 파생 Game의 멤버가 이미 파괴됐다. onExit을 호출하면 콜백이
    // 소멸한 멤버를 참조할 수 있으므로, 콜백 없이 Scene 소유권만 정리한다.
    scenes_.discard();
}

void Game::run() {
    Clock clock;
    run(clock);
}

void Game::run(Clock& clock) {
    // Application::isRunning()은 stop() 직후 false가 되므로 onStop/onExit까지
    // 포함한 전체 호출의 재진입 여부는 별도로 추적한다.
    if (runInvocationActive_) {
        throw std::logic_error("Game::run is already active");
    }
    RunInvocationGuard invocation{runInvocationActive_};
    try {
        Application::run(clock);
    } catch (...) {
        // 파생 클래스의 멤버가 살아 있는 동안 종료 콜백을 실행한다.
        // 정리 중 발생한 예외 대신 원래 실행 예외를 다시 던진다.
        try {
            (void)scenes_.shutdown();
        } catch (...) {
        }
        throw;
    }
}

void Game::processEvents() {
    window_.pollEvents();
    if (!window_.isOpen()) {
        stop();
    }
}

void Game::update(double fixedDeltaTime) { scenes_.update(fixedDeltaTime); }

void Game::render(double) {
    renderer_.beginFrame(clearColor_);
    scenes_.draw(renderer_);
    renderer_.endFrame();
}

void Game::onStop() {
    // 정상 종료에서는 onExit 예외를 호출자에게 전달한다. shutdown은 예외가
    // 발생하더라도 장면과 전환 예약을 모두 정리한 뒤 예외를 다시 던진다.
    (void)scenes_.shutdown();
}

} // namespace meno
