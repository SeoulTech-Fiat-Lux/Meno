#ifndef MENO_COLLIDER_H
#define MENO_COLLIDER_H

#include <meno/math/Vec2.hpp>
#include <meno/core/Component.hpp>

namespace meno {

class GameObject; //전방선언


/// <summary>
/// Collider 기본 골격의 구조체입니다. 본 구조체를 상속받아 새로운 Collider를 만들 수 있습니다.
/// </summary>
struct Collider : public Component {
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
struct BoxCollider : public Collider {    
    Vec2f size{};

    BoxCollider() = default;
    BoxCollider(const Vec2f& offset, const Vec2f& size) : Collider(offset), size(size) {}
};

/// <summary>
/// 기본 원형 Collider 입니다
/// </summary>
struct CircleCollider : public Collider {
    float radius{};

    CircleCollider() = default;
    CircleCollider(const Vec2f& offset, float radius) : Collider (offset), radius(radius) {}
};

/// <summary>
/// 두 game object가 가지고 있는 collider 간 충돌 판정을 수행합니다
/// 위치는 부모의 Transform::pos + offset이며, 사각형은 좌상단, 원은 중심 기준입니다.
/// 사각형은 축 정렬이며 Transform의 회전과 배율은 적용하지 않습니다.
/// 경계만 접하거나 크기/반지름이 0 이하인 경우 충돌하지 않습니다.
/// 등록된 BoxCollider/CircleCollider 조합 중 하나라도 겹치면 true입니다.
/// isTrigger와 layer는 이 기하 판정에서 필터링하지 않습니다.
/// </summary>
/// <returns>두 collider가 충돌될 시 true</returns>
bool intersects(const GameObject& a, const GameObject& b);

} // namespace meno
#endif
