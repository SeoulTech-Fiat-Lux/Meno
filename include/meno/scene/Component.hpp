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
    /// 복사로 생성한 컴포넌트는 부모 없이 시작한다.
    Component(const Component&) noexcept {}
    /// 부모는 복사하지 않으며 대입 대상의 기존 소유 관계를 유지한다.
    Component& operator=(const Component&) noexcept { return *this; }

    [[nodiscard]] GameObject* parent() const noexcept { return parent_; }

    virtual ~Component() = default;
};
} // namespace meno
#endif // MENO_COMPONENT_HPP
