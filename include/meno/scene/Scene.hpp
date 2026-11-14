#ifndef MENO_SCENE_HPP
#define MENO_SCENE_HPP

#include <meno/scene/GameObject.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace meno {

/// \addtogroup scene
/// \{

class SceneManager;

/// GameObject를 담는 장면입니다. 타이틀, 게임, 게임 오버처럼 화면 단위로 하나씩 만듭니다.
///
/// SceneManager::createScene()으로만 만들며, 한 번에 하나의 Scene만 활성 상태입니다. 활성 Scene의
/// 동작은 핸들러 네 개로 정합니다. enter 핸들러는 활성화될 때, update 핸들러는 fixed update마다,
/// draw 핸들러는 그릴 때, exit 핸들러는 비활성화될 때 호출됩니다. Scene은 GameObject를 레이어
/// 순서로 그린 뒤에 draw 핸들러를 호출하므로, 점수와 같은 텍스트나 도형은 draw 핸들러에서
/// 그립니다.
///
/// 주의할 점:
/// - update 핸들러 안에서 만든 GameObject는 바로 쓸 수 있지만, 그 update가 끝난 뒤부터 그려집니다.
/// - update 핸들러 안에서 지운 GameObject는 그 update가 끝날 때까지 남아 있습니다. 핸들러 밖에서
///   지우면 즉시 파괴되므로, 그 객체를 가리키던 포인터와 참조도 바로 무효가 됩니다.
///
/// \code{.cpp}
/// meno::Scene& title = game.scenes().createScene();
/// meno::Scene& play = game.scenes().createScene();
/// title.setUpdateHandler([&](double) {
///     if (game.window().input().wasPressed(meno::Key::Enter)) {
///         (void)game.scenes().changeScene(play.id());
///     }
/// });
/// \endcode
///
/// \see SceneManager, GameObject
class Scene {
    friend class SceneManager;

public:
    /// Scene의 ID 타입입니다. 같은 SceneManager 안에서 겹치지 않습니다.
    using SceneID = std::uint64_t;

private:
    SceneID id_;

    explicit Scene(SceneID id) : id_(id) {}
    ~Scene() = default;

    using ObjectsContainerType =
        std::unordered_map<GameObject::GameObjectID, std::unique_ptr<GameObject>>;
    ObjectsContainerType objects_;
    ObjectsContainerType pendingObjects_;
    bool updating_{false};
    bool pendingDeletions_{false};
    void applyObjectChanges();

    mutable std::vector<const GameObject*> drawOrder_;
    mutable bool drawOrderDirty_{true};
    GameObject::GameObjectID nextId_{1};

    using EnterHandler = std::function<void()>;
    using UpdateHandler = std::function<void(double)>;
    using ExitHandler = std::function<void()>;
    using DrawHandler = std::function<void(Renderer&)>;

    EnterHandler enterHandler_;
    UpdateHandler updateHandler_;
    ExitHandler exitHandler_;
    DrawHandler drawHandler_;

    using StateDeleter = void (*)(void*);
    std::unique_ptr<void, StateDeleter> state_{nullptr, +[](void*) {}};
    const std::type_info* stateType_{nullptr};

public:
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    /// 이 Scene의 ID입니다. SceneManager::changeScene()에 넘깁니다.
    [[nodiscard]] SceneID id() const noexcept { return id_; }

    /// 이 Scene이 활성화될 때 호출할 함수를 정합니다.
    ///
    /// 핸들러가 던진 예외는 그대로 전파되지만, 이 Scene은 활성 상태로 남습니다.
    void setEnterHandler(EnterHandler handler) { enterHandler_ = std::move(handler); }

    /// 이 Scene이 활성인 동안 fixed update마다 호출할 함수를 정합니다. 인자는 시간 간격(초)입니다.
    ///
    /// 핸들러가 예외를 던져도 그 안에서 만들거나 지운 GameObject를 반영한 뒤 예외를 전파합니다.
    void setUpdateHandler(UpdateHandler handler) { updateHandler_ = std::move(handler); }

    /// 이 Scene이 비활성화될 때(다른 Scene으로 전환하거나 SceneManager::shutdown()할 때) 호출할
    /// 함수를 정합니다.
    ///
    /// 핸들러가 예외를 던지면 전환이 취소되어 이 Scene이 활성 상태로 남습니다.
    void setExitHandler(ExitHandler handler) { exitHandler_ = std::move(handler); }

    /// 이 Scene을 그릴 때 GameObject를 모두 그린 뒤에 호출할 함수를 정합니다. 점수와 같은 텍스트나
    /// 도형은 인자로 받은 Renderer로 그립니다.
    ///
    /// 그린 순서대로 쌓이므로 여기서 그린 것은 GameObject보다 위에 보입니다. Game을 쓰면
    /// 핸들러 안에서 바꾼 카메라는 핸들러가 끝난 뒤 원래대로 돌아오므로, 화면 고정 UI는 월드
    /// 카메라와 size가 같고 움직이지 않는 카메라로 바꾼 뒤에 그리면 됩니다. 게임 상태는 update
    /// 핸들러에서 바꾸고 여기서는 그리기만 합니다. SceneManager::draw()로 그리는 동안에는 Scene
    /// 전환이 다음 update()로 미뤄집니다.
    ///
    /// \code{.cpp}
    /// const meno::Camera2D hud{.center = {400.f, 300.f}, .size = {800.f, 600.f}};
    /// play.setDrawHandler([&](meno::Renderer& renderer) {
    ///     renderer.setCamera(hud); // UI를 게임 화면에 고정합니다.
    ///     renderer.drawText(*font, "점수: " + std::to_string(score), {16.f, 16.f});
    /// });
    /// \endcode
    void setDrawHandler(DrawHandler handler) { drawHandler_ = std::move(handler); }

private:
    // SceneManager의 재진입 가드 안에서만 핸들러를 실행한다.
    void onEnter() const {
        if (enterHandler_)
            enterHandler_();
    }

    void onUpdate(double dt);

    void onExit() const {
        if (exitHandler_)
            exitHandler_();
    }

public:
    /// GameObject를 만들어 이 Scene에 넣고 그 참조를 돌려줍니다. 참조와 ID는 바로 쓸 수 있습니다.
    ///
    /// update 핸들러 안에서 만들면 그 update가 끝난 뒤부터(예외로 끝나도) 그려집니다. 참조는 그
    /// GameObject가 지워지거나 Scene이 사라질 때까지 유효합니다.
    GameObject& createGameObject();

    /// ID로 GameObject를 찾습니다. 없으면 nullptr입니다.
    /// update 중 삭제를 예약한 GameObject도 그 update가 끝날 때까지는 찾을 수 있습니다.
    /// \{
    [[nodiscard]]
    GameObject* getGameObject(GameObject::GameObjectID) noexcept;
    [[nodiscard]]
    const GameObject* getGameObject(GameObject::GameObjectID) const noexcept;
    /// \}
    /// GameObject를 지웁니다. 없는 ID나 이미 삭제를 예약한 ID면 false입니다.
    ///
    /// update 핸들러 안에서는 삭제를 예약만 하고 그 update가 끝날 때 지웁니다. 그 밖에서는 바로
    /// 파괴합니다.
    [[nodiscard]]
    bool deleteGameObject(GameObject::GameObjectID) noexcept;

    /// 그리기 레이어를 바꿉니다. 작은 레이어가 먼저(아래에) 그려지며 기본값은 0입니다.
    /// 없는 ID면 false입니다.
    [[nodiscard]] bool setLayer(GameObject::GameObjectID id, int layer) noexcept;

    /// GameObject를 작은 레이어부터, 같은 레이어는 만든 순서(ID 순)대로 그리고, 마지막에 draw
    /// 핸들러를 호출합니다. GameObject에서는 Sprite만 그립니다. Game을 쓰면 Game이 호출합니다.
    void drawScene(Renderer& renderer) const;

    /// 이 Scene의 상태 객체를 등록합니다. 점수처럼 Scene마다 보관할 데이터를 넣습니다.
    ///
    /// 소유권을 Scene으로 옮기며, Scene이 사라질 때 함께 파괴합니다. 빈 unique_ptr을 넘기면
    /// 등록되지 않으며, 나중에 다시 등록할 수 있습니다.
    ///
    /// \throws std::logic_error 이미 상태가 있을 때. 기존 상태는 그대로 두고 넘겨받은 객체는
    /// 파괴합니다.
    template <typename T>
    void setState(std::unique_ptr<T> state) {
        if (state_) {
            throw std::logic_error("Scene state already exists");
        }

        stateType_ = &typeid(T);
        state_ = decltype(state_){state.release(), +[](void* ptr) { delete static_cast<T*>(ptr); }};
    }

    /// 등록한 상태 객체를 소유권 이전 없이 꺼냅니다.
    ///
    /// 등록하지 않았거나 T가 setState()에 쓴 타입과 다르면 nullptr입니다. 돌려받은 포인터는 Scene이
    /// 사라지기 전까지 유효하며, delete해서는 안 됩니다. const Scene에서는 const T*를 돌려줍니다.
    /// \{
    template <typename T>
    T* getState() noexcept {
        if (!state_ || *stateType_ != typeid(T))
            return nullptr;
        return static_cast<T*>(state_.get());
    }

    template <typename T>
    const T* getState() const noexcept {
        if (!state_ || *stateType_ != typeid(T))
            return nullptr;
        return static_cast<const T*>(state_.get());
    }
    /// \}
};

/// \}

} // namespace meno

#endif // MENO_SCENE_HPP
