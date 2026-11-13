#ifndef MENO_SCENE_HPP
#define MENO_SCENE_HPP

#include <meno/scene/GameObject.hpp>

#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <typeinfo>
#include <utility>
#include <cstdint>


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

    using ObjectsContainerType = std::unordered_map<
        GameObject::GameObjectID, std::unique_ptr<GameObject>
    >;
    ObjectsContainerType objects_;
    ObjectsContainerType pending_objects_;
    bool updating_{false};
    bool pending_deletions_{false};
    void applyObjectChanges();

    mutable std::vector<const GameObject*> draw_order_;
    mutable bool draw_order_dirty_{true};
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
    const std::type_info* state_type_{nullptr};

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

    void onUpdate(double dt);

    void onExit() const {
        if (exit_handler_)
            exit_handler_();
    }

public:
    /// 업데이트 중에는 등록을 예약한다. 반환 참조와 ID로 즉시 설정/조회할 수 있다.
    /// 그리기 목록에는 업데이트 종료 시(예외 발생 시도 포함) 추가한다.
    GameObject& createGameObject() {
        const GameObject::GameObjectID id = next_id_++;

        std::unique_ptr<GameObject> object{
            new GameObject(id)
        };

        GameObject& ref = *object;
        if (updating_) {
            pending_objects_.emplace(id, std::move(object));
        } else {
            objects_.emplace(id, std::move(object));
            draw_order_dirty_ = true;
        }

        return ref;
    }

    [[nodiscard]]
    GameObject* getGameObject(GameObject::GameObjectID) noexcept;
    [[nodiscard]]
    const GameObject* getGameObject(GameObject::GameObjectID) const noexcept;
    /// 업데이트 중에는 삭제 표시만 한다. 객체와 조회 결과는 종료까지 유지한다.
    /// 없는 ID 또는 이미 삭제 예약된 ID는 false를 반환한다.
    [[nodiscard]]
    bool deleteGameObject(GameObject::GameObjectID) noexcept;

    /// 객체가 없으면 false. 실제 레이어 변경 시에만 그리기 순서를 무효화한다.
    [[nodiscard]] bool setLayer(GameObject::GameObjectID id, int layer) noexcept;

    /// 작은 레이어부터, 같은 레이어는 ID 순으로 그린다.
    void drawScene(Renderer& renderer) const {
        if (draw_order_dirty_) {
            draw_order_.clear();
            draw_order_.reserve(objects_.size());
            for (const auto& [id, object] : objects_)
                draw_order_.push_back(object.get());

            std::sort(draw_order_.begin(), draw_order_.end(),
                [](const GameObject* a, const GameObject* b) {
                    if (a->layer() != b->layer())
                        return a->layer() < b->layer();
                    return a->id() < b->id();
                });
            draw_order_dirty_ = false;
        }

        for (const auto* object : draw_order_)
            object->draw(renderer);
    }

    /// 상태 객체의 소유권을 Scene으로 이전하며, Scene 소멸 시 함께 파괴한다.
    /// 이미 유효한 상태가 있으면 std::logic_error를 던진다.
    /// 실패 시 기존 상태는 유지되며 전달받은 객체는 정리된다.
    /// 빈 unique_ptr은 상태를 등록하지 않으며 이후 다시 등록할 수 있다.
    template <typename T>
    void setState(std::unique_ptr<T> state) {
        if (state_) {
            throw std::logic_error("Scene state already exists");
        }

        state_type_ = &typeid(T);
        state_ = decltype(state_){
            state.release(),
            +[](void* ptr) {
                delete static_cast<T*>(ptr);
            }
        };
    }

    /// 소유권 이전 없이 상태를 조회한다. 미등록 상태에서는 nullptr을 반환한다.
    /// T는 setState에 사용한 타입이어야 하며 런타임 타입 검사는 하지 않는다.
    /// 반환 포인터는 Scene 소멸 전까지 유효하다. 호출자가 delete하면 안 된다.
    template <typename T>
    T* getState() noexcept {
        if (!state_ || *state_type_ != typeid(T))
            return nullptr;
        return static_cast<T*>(state_.get());
    }

    /// const Scene에서는 const T*를 반환하며, 타입과 수명 계약은 위와 같다.
    template <typename T>
    const T* getState() const noexcept {
        if (!state_ || *state_type_ != typeid(T))
            return nullptr;
        return static_cast<const T*>(state_.get());
    }
};

} // namespace meno

#endif  // MENO_SCENE_HPP
