#ifndef MENO_GAME_HPP
#define MENO_GAME_HPP

#include <meno/core/Application.hpp>
#include <meno/scene/SceneManager.hpp>
#include <meno/window/Window.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Vec2.hpp>
#include <meno/resources/ResourceManager.hpp>

#include <string>

namespace meno {

/// Game이 생성할 창과 게임 루프의 기본 설정.
struct GameConfig {
    ApplicationConfig application{};
    Vec2u windowSize{800u, 600u};
    std::string title{"meno"};
    Color clearColor{colors::Black};
};

/// 일반적인 Meno 게임의 창, 렌더러, 리소스와 장면 루프를 한곳에 묶는다.
///
/// 파생 클래스는 보통 onStart()에서 장면만 구성한다. 이벤트 처리, fixed update,
/// 장면 렌더링과 정상 종료 정리는 Game이 담당하며 재정의할 수 없다.
/// 각 Game은 독립적인 SceneManager를 직접 소유한다.
class Game : public Application {
public:
    explicit Game(GameConfig config = {});
    ~Game() noexcept override;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    /// Application 루프를 실행한다. 콜백이 예외를 던져도 파생 클래스의 멤버가
    /// 파괴되기 전에 Scene을 정리한 뒤 원래 예외를 다시 던진다.
    void run() final;
    void run(Clock& clock) final;

    [[nodiscard]] Window& window() noexcept { return window_; }
    [[nodiscard]] const Window& window() const noexcept { return window_; }

    [[nodiscard]] Renderer& renderer() noexcept { return renderer_; }
    [[nodiscard]] const Renderer& renderer() const noexcept { return renderer_; }

    [[nodiscard]] ResourceManager& resources() noexcept { return resources_; }
    [[nodiscard]] const ResourceManager& resources() const noexcept { return resources_; }

    [[nodiscard]] SceneManager& scenes() noexcept { return scenes_; }
    [[nodiscard]] const SceneManager& scenes() const noexcept { return scenes_; }

private:
    void processEvents() final;
    void update(double fixedDeltaTime) final;
    void render(double interpolationAlpha) final;
    void onStop() final;

    // 장면은 ~Game 본문에서 먼저 정리한다. 이후 resources -> renderer -> window
    // 순서로 소멸하므로 컴포넌트 소멸 중에도 이 의존성들을 사용할 수 있다.
    Window window_;
    Renderer renderer_;
    ResourceManager resources_;
    Color clearColor_;
    bool runInvocationActive_ = false;
    // 멤버 소멸 순서에서도 가장 먼저 정리되도록 마지막에 선언한다.
    SceneManager scenes_;
};

} // namespace meno

#endif // MENO_GAME_HPP
