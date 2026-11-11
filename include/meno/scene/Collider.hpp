#ifndef MENO_COLLIDER_H
#define MENO_COLLIDER_H

#include <meno/math/Vec2.hpp>
#include <meno/scene/Component.hpp>

namespace meno {

class GameObject; //전방선언


/// <summary>
/// Collider 기본 골격의 구조체입니다.
/// </summary>
struct Collider : public Component {
    /// 부모 좌상단 기준 로컬 위치. 사각형은 좌상단, 원은 중심을 지정한다.
    Vec2f offset{};
    bool isTrigger{false}; 
    int layer{0};

    Collider() = default;
    explicit Collider(const Vec2f& offset) : offset(offset) {}
    Collider(const Collider& other)
        : offset(other.offset), isTrigger(other.isTrigger), layer(other.layer) {}
};

/// <summary>
/// 기본 사각형 Collider 입니다
/// </summary>
struct BoxCollider final : public Collider {    
    Vec2f size{};

    BoxCollider() = default;
    BoxCollider(const Vec2f& offset, const Vec2f& size) : Collider(offset), size(size) {}

    /// 월드 좌표의 점이 내부에 있는지 검사한다. 회전된 박스 축 기준 왼쪽/위는 포함하고 오른쪽/아래는 제외한다.
    /// 부모 Transform의 위치, 회전, 균일 배율(음수 포함)과 offset을 반영한다.
    /// 부모가 없거나 크기가 0 이하이거나 배율이 0/비유한이면 false이다.
    /// isTrigger와 layer는 필터링하지 않는다.
    [[nodiscard]] bool contains(Vec2f worldPoint) const;
};

/// <summary>
/// 기본 원형 Collider 입니다
/// </summary>
struct CircleCollider final : public Collider {
    float radius{};

    CircleCollider() = default;
    CircleCollider(const Vec2f& offset, float radius) : Collider (offset), radius(radius) {}
};

/// <summary>
/// 두 game object가 가지고 있는 collider 간 충돌 판정을 수행합니다
/// 부모 좌상단을 원점으로 offset과 크기에 Transform의 배율과 회전을 적용합니다.
/// 사각형은 회전된 사각형으로 판정하며 원의 반지름에는 배율의 절댓값을 적용합니다.
/// 음수 배율은 원점 기준 반전입니다. 0/비유한 배율은 충돌하지 않습니다.
/// 경계만 접하거나 로컬 크기/반지름이 0 이하인 경우 충돌하지 않습니다.
/// 등록된 BoxCollider/CircleCollider 조합 중 하나라도 겹치면 true입니다.
/// isTrigger와 layer는 이 기하 판정에서 필터링하지 않습니다.
/// </summary>
/// <returns>두 collider가 충돌될 시 true</returns>
bool intersects(const GameObject& a, const GameObject& b);

} // namespace meno
#endif
