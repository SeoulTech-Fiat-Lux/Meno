#ifndef MENO_SCENE
#define MENO_SCENE

#include <meno/scene/GameObject.hpp>

#include <memory>
#include <unordered_map>
#include <functional>

namespace meno {

class SceneManager;

class Scene {
    friend class SceneManager;
public:
    using SceneID = std::uint64_t;

private:
    SceneID id_;

    explicit Scene(SceneID id) : id_(id) {}
    ~Scene() = default;

    std::unordered_map<
        GameObject::GameObjectID, std::unique_ptr<GameObject>
    > objects_;
    GameObject::GameObjectID next_id_{1};

    using EnterHandler  = std::function<void()>;
    using UpdateHandler = std::function<void(double)>;
    using ExitHandler   = std::function<void()>;

    EnterHandler enter_handler_;
    UpdateHandler update_handler_;
    ExitHandler exit_handler_;

    using StateDeleter = void (*)(void*);
    std::unique_ptr<void, StateDeleter> state_{
        nullptr, +[](void*) {}
    };

public:
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    [[nodiscard]] SceneID id() const noexcept { return id_; }

    void setEnterHandler(EnterHandler handler) {
        enter_handler_ = std::move(handler);
    }

    void setUpdateHandler(UpdateHandler handler) {
        update_handler_ = std::move(handler);
    }

    void setExitHandler(ExitHandler handler) {
        exit_handler_ = std::move(handler);
    }

private:
    // SceneManager의 재진입 가드 안에서만 핸들러를 실행한다.
    void onEnter() const {
        if (enter_handler_)
            enter_handler_();
    }

    void onUpdate(double dt) const {
        if (update_handler_)
            update_handler_(dt);
    }

    void onExit() const {
        if (exit_handler_)
            exit_handler_();
    }

public:
    GameObject& createGameObject() {
        const GameObject::GameObjectID id = next_id_++;

        std::unique_ptr<GameObject> object{
            new GameObject(id)
        };

        GameObject& ref = *object;
        objects_.emplace(id, std::move(object));

        return ref;
    }

    [[nodiscard]]
    GameObject* getGameObject(GameObject::GameObjectID) noexcept;
    [[nodiscard]]
    const GameObject* getGameObject(GameObject::GameObjectID) const noexcept;
    [[nodiscard]]
    bool deleteGameObject(GameObject::GameObjectID) noexcept;

    void drawScene(Renderer& renderer) const { 
        for (const auto& obj : objects_) {
            obj.second->draw(renderer);
        }
    }

    template <typename T>
    void setState(std::unique_ptr<T> state) {
        if (state_) {
            throw std::logic_error("Scene state already exists");
        }

        state_ = decltype(state_){
            state.release(),
            +[](void* ptr) {
                delete static_cast<T*>(ptr);
            }
        };
    }
};

} // namespace meno

#endif //MENO_SCENE
