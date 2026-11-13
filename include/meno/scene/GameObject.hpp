#ifndef MENO_GAME_OBJECT_HPP
#define MENO_GAME_OBJECT_HPP

#include <meno/scene/Component.hpp>
#include <meno/scene/Sprite.hpp>
#include <meno/scene/Transform.hpp>

#include <concepts>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace meno {

class Scene;

/// Scene에 놓이는 게임 객체입니다. 위치(Transform)와 그림(Sprite)을 기본으로 가집니다.
///
/// Scene::createGameObject()로만 만들며 Scene이 소유합니다. 다른 기능은 addComponent()로 붙이고,
/// 붙인 컴포넌트는 GameObject가 소유합니다. 컴포넌트는 타입마다 하나씩만 가질 수 있고, 떼어 내는
/// 함수는 없습니다. 복사도 이동도 할 수 없습니다.
///
/// 주의할 점:
/// - getComponent()는 정확히 같은 타입만 찾습니다. BoxCollider를 붙였다면
///   getComponent<Collider>()는 nullptr입니다.
/// - 같은 타입을 다시 addComponent()하면 이전 컴포넌트는 파괴되므로, 이전 참조를 쓰면 안 됩니다.
///
/// \code{.cpp}
/// meno::GameObject& player = scene.createGameObject();
/// player.transform().position = {100.f, 200.f};
/// player.sprite().setTexture(resources.loadTexture("assets/player.png"));
/// player.addComponent<meno::BoxCollider>(meno::Vec2f{0.f, 0.f}, meno::Vec2f{32.f, 32.f});
/// \endcode
///
/// \see Scene, Component, Transform, Sprite
class GameObject final {
    friend class Scene;

public:
    /// GameObject의 ID 타입입니다. 같은 Scene 안에서 겹치지 않고, 만든 순서대로 커집니다.
    using GameObjectID = std::uint64_t;

private:
    int layer_{0};
    bool pendingDelete_{false};
    Transform transform_;
    Sprite sprite_;

    std::unordered_map<std::type_index, std::unique_ptr<Component>> components_;

    GameObjectID id_;

    GameObject(GameObjectID id) : id_(id) {
        transform_.parent_ = this;
        sprite_.parent_ = this;
    }

public:
    ~GameObject() = default;
    GameObject(const GameObject&) = delete;
    GameObject(GameObject&&) noexcept = delete;

    GameObject& operator=(const GameObject&) = delete;
    GameObject& operator=(GameObject&&) = delete;

    /// 기본 컴포넌트인 Transform입니다.
    /// \{
    Transform& transform() noexcept { return transform_; }
    const Transform& transform() const noexcept { return transform_; }
    /// \}

    /// 기본 컴포넌트인 Sprite입니다.
    /// \{
    Sprite& sprite() noexcept { return sprite_; }
    const Sprite& sprite() const noexcept { return sprite_; }
    /// \}

    /// Sprite를 현재 Transform으로 그립니다. 보통은 Scene::drawScene()이 부릅니다.
    void draw(Renderer& renderer) const { sprite_.draw(renderer); }
    /// 이 GameObject의 ID입니다. Scene::getGameObject()로 다시 찾을 때 씁니다.
    [[nodiscard]] GameObjectID id() const noexcept { return id_; }
    /// 그리기 레이어입니다. 기본값은 0이며 Scene::setLayer()로 바꿉니다.
    [[nodiscard]] int layer() const noexcept { return layer_; }

    /// T 타입 컴포넌트를 만들어 붙이고, 그 참조를 돌려줍니다. args는 T의 생성자에 그대로
    /// 넘깁니다.
    ///
    /// 같은 타입이 이미 있으면 이전 컴포넌트를 파괴하고 새것으로 바꿉니다. Transform과 Sprite는
    /// 기본 컴포넌트이므로 붙일 수 없습니다(컴파일 오류).
    template <typename T, typename... Args>
        requires std::derived_from<T, Component> && (!std::is_same_v<T, Transform>) &&
                 (!std::is_same_v<T, Sprite>)
    T& addComponent(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        ptr->parent_ = this;

        T& ref = *ptr;

        components_[std::type_index(typeid(T))] = std::move(ptr);

        return ref;
    }

    /// 붙어 있는 T 타입 컴포넌트를 찾습니다. 없으면 nullptr입니다.
    ///
    /// 정확히 같은 타입만 찾으므로 기반 타입으로는 찾을 수 없습니다. Transform과 Sprite는
    /// transform()과 sprite()로 얻습니다.
    /// \{
    template <typename T>
        requires std::derived_from<T, Component> && (!std::is_same_v<T, Transform>) &&
                 (!std::is_same_v<T, Sprite>)
    T* getComponent() noexcept {
        return const_cast<T*>(std::as_const(*this).template getComponent<T>());
    }

    template <typename T>
        requires std::derived_from<T, Component> && (!std::is_same_v<T, Transform>) &&
                 (!std::is_same_v<T, Sprite>)
    const T* getComponent() const noexcept {
        auto it = components_.find(std::type_index(typeid(T)));

        if (it == components_.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }
    /// \}
};

} // namespace meno

#endif // MENO_GAME_OBJECT_HPP
