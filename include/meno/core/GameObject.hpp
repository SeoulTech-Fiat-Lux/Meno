//
// Created by 최상준 on 26. 8. 6..
//

#ifndef MENO_GAMEOBJECT_HPP
#define MENO_GAMEOBJECT_HPP

#include <meno/math/Vec2.hpp>
#include <meno/core/Collider.hpp>
#include <meno/core/Component.hpp>

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <type_traits>

namespace meno {

struct Transform : public Component {
    Vec2f pos {0,0};       // 2D 위치 (x, y), 좌상단부터 (0, 0)
    int magnitude{1};  // 크기 배율
    int rotation{0};       // 360도 기준 회전
    int width{0}, height{0}; // 크기, 너비
};

// 렌더링 파이프라인 참고해서 수정 필요
struct Sprite : public Component {
    int width, height; // 크기, 너비
    int* pixels;       // 픽셀 데이터
};

class Scene; // friend 선언을 위한 전방선언

/**
 * @brief 게임 내 객체를 표현하는 기본 클래스입니다.
 *
 * 모든 GameObject는 Transform과 Sprite를 기본 컴포넌트로 가지며,
 * 추가적인 컴포넌트는 타입을 기준으로 동적으로 등록하고 조회할 수 있습니다.
 * 같은 타입의 컴포넌트는 중복되어 소유할 수 없으며, 마지막으로 등록된 것을 소유합니다.
 *
 * 등록된 컴포넌트는 GameObject가 독점적으로 소유하며,
 * GameObject 간 복사는 허용하지 않고 이동만 허용합니다.
 */
class GameObject final {
    using GameObjectID = std::uint64_t;

    friend class Scene;

private:
// private 맴버변수 필드
    Transform transform_;
    Sprite sprite_;

    std::unordered_map<std::type_index, std::unique_ptr<Component>> components_;

    GameObjectID id_;

// prvate 메서드 필드
    GameObject() = default;
    GameObject(const GameObject&) = delete;
    GameObject(GameObject&& other) noexcept = default;

    GameObject(GameObjectID id) : id_(id) {}

public:
    GameObject& operator=(const GameObject&) = delete;
    GameObject& operator=(GameObject&&) = delete;

    Transform& transform() noexcept { return transform_; }
    GameObjectID id() noexcept { return id_; }

    /**
     * @brief 지정한 타입의 컴포넌트를 생성하여 GameObject에 등록합니다.
     *
     * 전달된 인자를 컴포넌트의 생성자에 완벽 전달(perfect forwarding)하여
     * 새로운 컴포넌트를 생성하고 해당 컴포넌트의 소유권을 GameObject에 귀속시킵니다.
     *
     * Transform과 Sprite는 GameObject의 기본 컴포넌트이므로
     * addComponent()를 통한 추가 등록을 허용하지 않습니다.
     *
     * 동일한 타입의 컴포넌트가 이미 존재하는 경우 기존 컴포넌트는
     * 제거되고 새로 생성된 컴포넌트로 교체됩니다.
     *
     * @tparam T 등록할 컴포넌트의 타입입니다.
     * @tparam Args 컴포넌트 생성자에 전달할 인자의 타입입니다.
     * @param args 컴포넌트 생성자에 전달할 인자입니다.
     * @return 새로 등록된 컴포넌트에 대한 참조입니다.
     */
    template <typename T, typename... Args>
        requires std::derived_from<T, Component> && 
        (!std::is_same_v<T, Transform>) && (!std::is_same_v<T, Sprite>)
    T& addComponent(Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);

        T& ref = *ptr;

        components_[std::type_index(typeid(T))] = std::move(ptr);

        return ref;
    }

    /**
     * @brief 지정한 타입의 컴포넌트를 조회합니다.
     *
     * 등록된 컴포넌트 중 T와 정확히 일치하는 타입의 컴포넌트를 검색합니다.
     *
     * @tparam T 조회할 컴포넌트의 타입입니다.
     * @return 컴포넌트가 존재하면 해당 객체의 포인터를 반환하며,
     *         존재하지 않으면 nullptr을 반환합니다.
     */
    template <typename T>
        requires std::derived_from<T, Component>
    T* getComponent() noexcept {
        auto it = components_.find(std::type_index(typeid(T)));

        if (it == components_.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }

    template <typename T>
        requires std::derived_from<T, Component>
    const T* getComponent() const noexcept {
        auto it = components_.find(std::type_index(typeid(T)));

        if (it == components_.end())
            return nullptr;

        return static_cast<T*>(it->second.get());
    }
};

} // namespace meno

#endif // MENO_GAMEOBJECT_HPP