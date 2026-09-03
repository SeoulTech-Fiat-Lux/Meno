#ifndef MENO_SCENE
#define MENO_SCENE

#include <meno/core/GameObject.hpp>

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace meno {

class Scene {

private:
    std::unordered_map<
        GameObject::GameObjectID, std::unique_ptr<GameObject>
    > objects_;
    GameObject::GameObjectID next_id_{1};

    using EnterHandler  = std::function<void()>;
    using UpdateHandler = std::function<void(float)>;
    using ExitHandler   = std::function<void()>;

    EnterHandler enter_handler_;
    UpdateHandler update_handler_;
    ExitHandler exit_handler_;

public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    void setEnterHandler(EnterHandler handler) {
        enter_handler_ = std::move(handler);
    }

    void setUpdateHandler(UpdateHandler handler) {
        update_handler_ = std::move(handler);
    }

    void setExitHandler(ExitHandler handler) {
        exit_handler_ = std::move(handler);
    }

    void onEnter() const {
        if (enter_handler_)
            enter_handler_();
    }

    void onUpdate(float dt) const {
        if (update_handler_)
            update_handler_(dt);
    }

    void onExit() const {
        if (exit_handler_)
            exit_handler_();
    }

    template <typename T = GameObject, typename... Args>
        requires std::derived_from<T, GameObject>
    T& createGameObject(Args&&... args) {
        auto object = std::make_unique<T>(next_id_, std::forward<Args>(args)...);

        T& ref = *object;
        objects_.emplace(next_id_++, std::move(object));

        return ref;
    }

    [[nodiscard]]
    GameObject* getGameObject(GameObject::GameObjectID) noexcept;
    [[nodiscard]]
    const GameObject* getGameObject(GameObject::GameObjectID) const noexcept;

    [[nodiscard]]
    bool deleteGameObject(GameObject::GameObjectID) noexcept;
};

} // namespace meno

#endif //MENO_SCENE