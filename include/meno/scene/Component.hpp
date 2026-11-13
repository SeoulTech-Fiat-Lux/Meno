#ifndef MENO_COMPONENT_HPP
#define MENO_COMPONENT_HPP

namespace meno {
class GameObject;

/// 모든 component의 기본 구조체입니다. 본 구조체를 상속받아 새로운 component를 만들 수 있습니다.
struct Component {
private:
    friend class GameObject;
    GameObject* parent_{nullptr};

public:
    Component() = default;
    /// 복사·이동으로 생성한 컴포넌트는 부모 없이 시작한다.
    /// \{
    Component(const Component& /*other*/) noexcept {}
    Component(Component&& /*other*/) noexcept {}
    /// \}

    /// 부모는 복사하지 않으며 대입 대상의 기존 소유 관계를 유지한다.
    /// 아무것도 복사하지 않으므로 자기 대입도 안전하다.
    /// \{
    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
    Component& operator=(const Component& /*other*/) noexcept { return *this; }
    Component& operator=(Component&& /*other*/) noexcept { return *this; }
    /// \}

    /// 이 컴포넌트를 가진 GameObject. 아직 붙지 않았으면 nullptr.
    /// \{
    [[nodiscard]] GameObject* parent() noexcept { return parent_; }
    [[nodiscard]] const GameObject* parent() const noexcept { return parent_; }
    /// \}

    virtual ~Component() = default;
};
} // namespace meno
#endif // MENO_COMPONENT_HPP
