#include <meno/core/Game.hpp>

#include <stdexcept>

namespace meno {
namespace {

Game* liveGame = nullptr;

struct RunInvocationGuard {
    bool& active;

    explicit RunInvocationGuard(bool& flag) : active(flag) { active = true; }
    ~RunInvocationGuard() { active = false; }
};

} // namespace

Game::Game(GameConfig config)
    : Application(config.application),
      window_(config.windowSize, config.title),
      renderer_(window_),
      scenes_(SceneManager::instance()),
      clearColor_(config.clearColor) {
    if (liveGame != nullptr) {
        throw std::logic_error("Only one Game instance may exist at a time");
    }
    liveGame = this;
}

Game::~Game() noexcept {
    // 이 시점에는 파생 Game의 멤버가 이미 파괴됐다. onExit을 호출하면 콜백이
    // 소멸한 멤버를 참조할 수 있으므로, 콜백 없이 Scene 소유권만 정리한다.
    scenes_.discard();
    liveGame = nullptr;
}

void Game::run() {
    // Application::isRunning()은 stop() 직후 false가 되므로 onStop/onExit까지
    // 포함한 전체 호출의 재진입 여부는 별도로 추적한다.
    if (runInvocationActive_) {
        throw std::logic_error("Game::run is already active");
    }
    RunInvocationGuard invocation{runInvocationActive_};
    try {
        Application::run();
    } catch (...) {
        // 이 catch는 파생 Game의 소멸이 시작되기 전에 실행된다. exit handler가
        // 파생 클래스 멤버를 참조하더라도 아직 유효한 시점에 정리한다.
        try {
            (void)scenes_.shutdown();
        } catch (...) {
        }
        throw;
    }
}

void Game::run(Clock& clock) {
    if (runInvocationActive_) {
        throw std::logic_error("Game::run is already active");
    }
    RunInvocationGuard invocation{runInvocationActive_};
    try {
        Application::run(clock);
    } catch (...) {
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

void Game::update(double fixedDeltaTime) {
    scenes_.update(fixedDeltaTime);
}

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
