#ifndef MENO_COLLIDER_HPP
#define MENO_COLLIDER_HPP

#include <meno/math/Vec2.hpp>
#include <meno/scene/Component.hpp>

namespace meno {

class GameObject;

/// 모든 콜라이더의 기본 구조체.
struct Collider : public Component {
    /// 부모 좌상단 기준 로컬 위치. 사각형은 좌상단, 원은 중심을 지정한다.
    Vec2f offset{};

    Collider() = default;
    explicit Collider(const Vec2f& offset) : offset(offset) {}
};

/// 사각형 콜라이더.
struct BoxCollider final : public Collider {
    Vec2f size{};

    BoxCollider() = default;
    BoxCollider(const Vec2f& offset, const Vec2f& size) : Collider(offset), size(size) {}

    /// 월드 좌표의 점이 내부에 있는지 검사한다. 회전된 박스 축 기준 왼쪽/위는 포함하고
    /// 오른쪽/아래는 제외한다. 부모 Transform의 위치, 회전, 균일 배율(음수 포함)과 offset을
    /// 반영한다. 부모가 없거나 크기가 0 이하이거나 배율이 0/비유한이면 false이다.
    [[nodiscard]] bool contains(Vec2f worldPoint) const;
};

/// 원형 콜라이더.
struct CircleCollider final : public Collider {
    float radius{};

    CircleCollider() = default;
    CircleCollider(const Vec2f& offset, float radius) : Collider(offset), radius(radius) {}
};

/// 두 GameObject에 등록된 콜라이더 간 충돌을 판정한다.
/// 부모 좌상단을 원점으로 offset과 크기에 Transform의 배율과 회전을 적용한다.
/// 사각형은 회전된 사각형으로 판정하며 원의 반지름에는 배율의 절댓값을 적용한다.
/// 음수 배율은 원점 기준 반전이다. 0/비유한 배율은 충돌하지 않는다.
/// 경계만 접하거나 로컬 크기/반지름이 0 이하인 경우 충돌하지 않는다.
/// 등록된 BoxCollider/CircleCollider 조합 중 하나라도 겹치면 true를 반환한다.
bool intersects(const GameObject& a, const GameObject& b);

} // namespace meno
#endif // MENO_COLLIDER_HPP
