#ifndef MENO_SCENEMANAGER_HPP
#define MENO_SCENEMANAGER_HPP

#include <meno/scene/Scene.hpp>
#include <optional>

namespace meno {

/// 단일 스레드 게임 루프에서 사용한다. 최초 접근 시 생성, 정상 종료 시 소멸한다.
class SceneManager final {
private:

    struct SceneDeleter {
        void operator()(Scene* scene) const { delete scene; }
    };

    std::unordered_map<Scene::SceneID, std::unique_ptr<Scene, SceneDeleter>> scenes_;
    Scene::SceneID nextId_{1};
    Scene* active_{nullptr};
    std::optional<Scene::SceneID> pending_;
    bool busy_{false};

    // private 맴버 함수 필드
    SceneManager() = default;
    ~SceneManager() = default;

    void applyPending();
    
public:
    static SceneManager& instance();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

    /// 비활성 Scene을 생성한다. 참조는 해당 Scene 삭제/shutdown까지 유효하다.
    Scene& createScene();
    [[nodiscard]] Scene* getScene(Scene::SceneID id) noexcept;
    [[nodiscard]] const Scene* getScene(Scene::SceneID id) const noexcept;
    [[nodiscard]] Scene* activeScene() noexcept { return active_; }
    [[nodiscard]] const Scene* activeScene() const noexcept { return active_; }

    /// 콜백 안에서는 예약하며 마지막 유효 요청이 우선한다.
    /// enter/exit에서 예약한 전환은 다음 update의 시작/종료 처리 시점에 적용한다.
    [[nodiscard]] bool changeScene(Scene::SceneID id);
    /// 활성 Scene 또는 콜백 실행 중에는 삭제할 수 없다.
    [[nodiscard]] bool deleteScene(Scene::SceneID id);
    void update(float dt);
    void draw(Renderer& renderer) {
        if (active_) active_->drawScene(renderer);
    }
    /// 창/오디오 시스템 종료 전에 호출한다. 콜백 안에서는 false를 반환한다.
    /// Scene을 모두 해제하지만 매니저는 유지되며 다시 사용할 수 있다.
    [[nodiscard]] bool shutdown();


};

} // namespace meno
#endif
