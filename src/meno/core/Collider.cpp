#include <meno/core/Collider.hpp>
#include <meno/core/GameObject.hpp>

#include <meno/math/Rect.hpp>

#include <algorithm>

namespace meno {

namespace {

bool intersects_box_circle(const meno::BoxCollider& box, const meno::CircleCollider& circle) {
    if (!box.parent || !circle.parent || box.size.x <= 0.f || box.size.y <= 0.f || circle.radius <= 0.f)
        return false;

    // 월드좌표로 변환
    const Rectf bounds{box.parent->transform().pos + box.offset, box.size};
    const Vec2f center = circle.parent->transform().pos + circle.offset;
    const Vec2f closest{
        std::clamp(center.x, bounds.left(), bounds.right()),
        std::clamp(center.y, bounds.top(), bounds.bottom())
    };
    return (center - closest).lengthSquared() < circle.radius * circle.radius;
}

bool intersects_box_box(const meno::BoxCollider& a, const meno::BoxCollider& b) {
    if (!a.parent || !b.parent || a.size.x <= 0.f || a.size.y <= 0.f || b.size.x <= 0.f || b.size.y <= 0.f)
        return false;

    const Rectf boundsA{a.parent->transform().pos + a.offset, a.size};
    const Rectf boundsB{b.parent->transform().pos + b.offset, b.size};
    return boundsA.intersects(boundsB);
}

bool intersects_circle_circle(const meno::CircleCollider& a, const meno::CircleCollider& b) {
    if (!a.parent || !b.parent || a.radius <= 0.f || b.radius <= 0.f)
        return false;

    const Vec2f centerA = a.parent->transform().pos + a.offset;
    const Vec2f centerB = b.parent->transform().pos + b.offset;
    const float radiusSum = a.radius + b.radius;
    return (centerA - centerB).lengthSquared() < radiusSum * radiusSum;
}

} // namespace

bool intersects(const GameObject& a, const GameObject& b) {

    if (const auto* boxA = a.getComponent<BoxCollider>()) {

        if (const auto* boxB = b.getComponent<BoxCollider>()) {
            if (intersects_box_box(*boxA, *boxB))
                return true;
        }

        if (const auto* circleB = b.getComponent<CircleCollider>()) {
            if (intersects_box_circle(*boxA, *circleB))
                return true;
        }
    }

    if (const auto* circleA = a.getComponent<CircleCollider>()) {

        if (const auto* boxB = b.getComponent<BoxCollider>()) {
            if (intersects_box_circle(*boxB, *circleA))
                return true;
        }

        if (const auto* circleB = b.getComponent<CircleCollider>()) {
            if (intersects_circle_circle(*circleA, *circleB))
                return true;
        }
    }

    return false;
}

} // namespace meno
