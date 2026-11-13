#ifndef MENO_GAME_HPP
#define MENO_GAME_HPP

#include <meno/core/Application.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Vec2.hpp>
#include <meno/resources/ResourceManager.hpp>
#include <meno/scene/SceneManager.hpp>
#include <meno/window/Window.hpp>

#include <string>

namespace meno {

/// Game이 만들 창과 게임 루프의 설정입니다.
///
/// 필드는 선언 순서(application, windowSize, title, clearColor)대로 지정해야 합니다.
struct GameConfig {
    /// 게임 루프의 시간 설정입니다.
    ApplicationConfig application{};
    /// 창 안쪽(그리기 영역)의 픽셀 크기입니다.
    Vec2u windowSize{800u, 600u};
    /// 창 제목입니다. UTF-8로 해석합니다.
    std::string title{"meno"};
    /// 프레임마다 화면을 지울 색입니다.
    Color clearColor{colors::Black};
};

/// 창, 렌더러, 리소스, 장면 루프를 한곳에 묶은 게임입니다. 보통의 게임은 이 클래스로 만듭니다.
///
/// 만들면 바로 창이 뜨고, run()을 부르면 창을 닫을 때까지 루프를 돕니다. 프레임마다 입력을
/// 처리하고(창이 닫히면 멈춤), 활성 Scene의 update 핸들러를 부르고, 활성 Scene을 그립니다. 이
/// 흐름은 재정의할 수 없으며, 상속한다면 onStart()에서 Scene을 구성합니다. 각 Game은 자기
/// SceneManager를 소유합니다.
///
/// 주의할 점:
/// - Game은 활성 Scene에 있는 GameObject의 Sprite만 그립니다. 점수 같은 텍스트나 도형을 그릴 곳이
///   없으므로, 그런 그리기가 필요하면 Application을 직접 상속해 render()에서 Renderer로 그립니다.
/// - 한 번에 하나의 Application(Game 포함)만 실행할 수 있습니다.
///
/// \code{.cpp}
/// meno::Game game{{.windowSize = {640, 480}, .title = "Pong"}};
/// meno::Scene& play = game.scenes().createScene();
/// play.setUpdateHandler([&](double fixedDeltaTime) {
///     // 게임 상태를 fixedDeltaTime(초)만큼 진행합니다.
/// });
/// (void)game.scenes().changeScene(play.id());
/// game.run(); // 창을 닫으면 돌아옵니다.
/// \endcode
///
/// \see Application, SceneManager, GameConfig
class Game : public Application {
public:
    /// config대로 창과 렌더러를 만듭니다. 창은 이때 바로 뜹니다.
    /// \throws std::invalid_argument config.application이 잘못되었을 때(Application 참고)
    explicit Game(const GameConfig& config = {});
    /// 남은 Scene을 exit 핸들러 없이 정리한 뒤 파괴합니다. run()이 정상적으로 끝났다면 exit
    /// 핸들러는 이미 불렸습니다.
    ~Game() noexcept override;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    /// 게임 루프를 실행합니다. 창을 닫거나 stop()을 부르면 활성 Scene의 exit 핸들러를 부르고
    /// 돌아옵니다. Clock을 넘기면 그 시계로 시간을 잽니다(테스트용).
    ///
    /// 핸들러가 예외를 던지면 Scene을 정리(exit 핸들러 포함)한 뒤 원래 예외를 다시 던집니다.
    ///
    /// \throws std::logic_error 이 Game이 이미 run() 중이거나 다른 Application이 실행 중일 때
    /// \{
    void run() final;
    void run(Clock& clock) final;
    /// \}

    /// 게임 창입니다. 입력은 window().input()으로 읽습니다.
    /// \{
    [[nodiscard]] Window& window() noexcept { return window_; }
    [[nodiscard]] const Window& window() const noexcept { return window_; }
    /// \}

    /// 창에 그리는 Renderer입니다. Game에서는 주로 카메라를 정할 때(setCamera) 씁니다.
    /// \{
    [[nodiscard]] Renderer& renderer() noexcept { return renderer_; }
    [[nodiscard]] const Renderer& renderer() const noexcept { return renderer_; }
    /// \}

    /// 리소스 캐시입니다. 텍스처, 폰트, 효과음을 읽을 때 씁니다.
    /// \{
    [[nodiscard]] ResourceManager& resources() noexcept { return resources_; }
    [[nodiscard]] const ResourceManager& resources() const noexcept { return resources_; }
    /// \}

    /// 이 Game의 SceneManager입니다.
    /// \{
    [[nodiscard]] SceneManager& scenes() noexcept { return scenes_; }
    [[nodiscard]] const SceneManager& scenes() const noexcept { return scenes_; }
    /// \}

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
