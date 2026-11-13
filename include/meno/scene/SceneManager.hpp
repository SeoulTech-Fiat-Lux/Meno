#ifndef MENO_SCENE_MANAGER_HPP
#define MENO_SCENE_MANAGER_HPP

#include <meno/scene/Scene.hpp>
#include <memory>
#include <optional>
#include <unordered_map>

namespace meno {

/// \addtogroup scene
/// \{

class Game;

/// Scene을 만들고, 그중 하나를 활성 Scene으로 돌립니다.
///
/// Game을 쓰면 Game이 하나를 소유하고(Game::scenes()), update(), draw(), 종료할 때의 shutdown()을
/// 대신 부릅니다. 단일 스레드 게임 루프에서만 씁니다.
///
/// 주의할 점:
/// - 소멸할 때는 exit 핸들러를 부르지 않습니다. 종료 처리가 필요하면 먼저 shutdown()을 부릅니다.
/// - Scene의 핸들러 안에서는 전환이 바로 일어나지 않고 예약됩니다(changeScene() 참고).
///
/// \code{.cpp}
/// meno::SceneManager& scenes = game.scenes();
/// meno::Scene& title = scenes.createScene();
/// (void)scenes.changeScene(title.id()); // 핸들러 밖이므로 바로 전환합니다.
/// \endcode
///
/// \see Scene, Game::scenes
class SceneManager final {
private:
    friend class Game;

    struct SceneDeleter {
        void operator()(Scene* scene) const { delete scene; }
    };

    std::unordered_map<Scene::SceneID, std::unique_ptr<Scene, SceneDeleter>> scenes_;
    Scene::SceneID nextId_{1};
    Scene* active_{nullptr};
    std::optional<Scene::SceneID> pending_;
    bool busy_{false};
    void applyPending();
    // Game의 마지막 소멸 방어선. 파생 Game이 이미 소멸한 시점이므로
    // 사용자 onExit 콜백은 호출하지 않고 소유한 Scene만 폐기한다.
    void discard() noexcept;

public:
    /// Scene이 없는 매니저를 만듭니다.
    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    /// 비활성 Scene을 만듭니다. 참조는 그 Scene을 deleteScene()하거나 shutdown()할 때까지
    /// 유효합니다.
    [[nodiscard]] Scene& createScene();
    /// ID로 Scene을 찾습니다. 없으면 nullptr입니다.
    /// \{
    [[nodiscard]] Scene* getScene(Scene::SceneID id) noexcept;
    [[nodiscard]] const Scene* getScene(Scene::SceneID id) const noexcept;
    /// \}
    /// 지금 활성인 Scene입니다. 없으면 nullptr입니다.
    /// \{
    [[nodiscard]] Scene* activeScene() noexcept { return active_; }
    [[nodiscard]] const Scene* activeScene() const noexcept { return active_; }
    /// \}

    /// id의 Scene으로 전환합니다. 없는 ID면 false입니다.
    ///
    /// Scene의 핸들러 밖에서 부르면 바로 전환합니다(이전 Scene의 exit, 새 Scene의 enter 순).
    /// update 핸들러 안에서 부르면 그 update가 끝난 직후에, enter·exit 핸들러 안에서 부르면 다음
    /// update()가 시작될 때 전환합니다. 여러 번 부르면 마지막 요청만 남습니다.
    [[nodiscard]] bool changeScene(Scene::SceneID id);
    /// Scene을 지웁니다. 그 Scene의 GameObject도 함께 파괴됩니다.
    ///
    /// 활성 Scene이거나 Scene의 핸들러 안이면 지우지 않고 false입니다. 없는 ID도 false입니다.
    [[nodiscard]] bool deleteScene(Scene::SceneID id);
    /// 예약된 전환을 반영하고, 활성 Scene의 update 핸들러를 부릅니다. Game을 쓰면 Game이 fixed
    /// update마다 부릅니다. Scene의 핸들러 안에서 부르면 아무것도 하지 않습니다.
    void update(double dt);
    /// 활성 Scene을 그립니다(Scene::drawScene()). Game을 쓰면 Game이 부릅니다.
    ///
    /// 그리는 동안(draw 핸들러 안 포함)은 Scene의 핸들러 안과 같이 취급합니다. changeScene()은
    /// 다음 update()가 시작될 때 반영되고, deleteScene()과 shutdown()은 false를 돌려줍니다.
    void draw(Renderer& renderer);
    /// 활성 Scene의 exit 핸들러를 부르고 모든 Scene을 지웁니다. 창이나 오디오가 정리되기 전에
    /// 부릅니다.
    ///
    /// Scene의 핸들러 안에서 부르면 아무것도 하지 않고 false입니다. 지운 뒤에도 매니저는 다시 쓸 수
    /// 있습니다. exit 핸들러가 예외를 던져도 모든 Scene과 예약된 전환을 정리한 뒤 예외를 다시
    /// 던집니다.
    [[nodiscard]] bool shutdown();
};

/// \}

} // namespace meno
#endif // MENO_SCENE_MANAGER_HPP
