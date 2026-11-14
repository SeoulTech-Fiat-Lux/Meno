#ifndef MENO_COMPONENT_HPP
#define MENO_COMPONENT_HPP

namespace meno {

/// \addtogroup scene
/// \{

class GameObject;

/// 모든 컴포넌트의 기반 구조체입니다. 상속해서 새 컴포넌트를 만들고 GameObject::addComponent()로
/// 붙입니다.
///
/// 컴포넌트에는 매 프레임 호출되는 함수가 없습니다. 동작은 Scene의 update 핸들러에서
/// GameObject::getComponent()로 컴포넌트를 꺼내서 처리합니다.
///
/// \code{.cpp}
/// struct Health : meno::Component {
///     int value = 3;
/// };
///
/// player.addComponent<Health>();
/// if (auto* health = player.getComponent<Health>()) {
///     health->value -= 1;
/// }
/// \endcode
///
/// \see GameObject::addComponent, GameObject::getComponent
struct Component {
private:
    friend class GameObject;
    GameObject* parent_{nullptr};

public:
    /// 부모 없는 컴포넌트를 만듭니다.
    Component() = default;
    /// 복사하거나 이동해 만든 컴포넌트는 부모 없이 시작합니다.
    /// \{
    Component(const Component& /*other*/) noexcept {}
    Component(Component&& /*other*/) noexcept {}
    /// \}

    /// 대입해도 부모는 복사하지 않고, 대입 대상이 원래 붙어 있던 GameObject를 유지합니다.
    /// 아무것도 복사하지 않으므로 자기 대입도 안전합니다.
    /// \{
    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
    Component& operator=(const Component& /*other*/) noexcept { return *this; }
    Component& operator=(Component&& /*other*/) noexcept { return *this; }
    /// \}

    /// 이 컴포넌트가 붙은 GameObject입니다. 아직 붙지 않았으면 nullptr입니다.
    /// \{
    [[nodiscard]] GameObject* parent() noexcept { return parent_; }
    [[nodiscard]] const GameObject* parent() const noexcept { return parent_; }
    /// \}

    virtual ~Component() = default;
};

/// \}

} // namespace meno
#endif // MENO_COMPONENT_HPP
