#include <meno/scene/Collider.hpp>
#include <meno/scene/GameObject.hpp>

#include <cmath>

#include <algorithm>

namespace meno {

namespace {

struct WorldBox {
    Vec2f center;
    Vec2f halfSize;
    Vec2f xAxis;
    Vec2f yAxis;
};

bool valid(const Collider& collider) {
    return collider.parent() && std::isfinite(collider.parent()->transform().scale) &&
           collider.parent()->transform().scale != 0.f;
}

bool valid(const BoxCollider& box) {
    return valid(static_cast<const Collider&>(box)) && box.size.x > 0.f && box.size.y > 0.f;
}

bool valid(const CircleCollider& circle) {
    return valid(static_cast<const Collider&>(circle)) && circle.radius > 0.f;
}

WorldBox worldBox(const BoxCollider& box) {
    const auto& transform = box.parent()->transform();
    return {transform.transformPoint(box.offset + box.size * 0.5f),
            box.size * (std::abs(transform.scale) * 0.5f), transform.rotateVector({1.f, 0.f}),
            transform.rotateVector({0.f, 1.f})};
}

float worldRadius(const CircleCollider& circle) {
    return circle.radius * std::abs(circle.parent()->transform().scale);
}

bool intersectsBoxCircle(const BoxCollider& box, const CircleCollider& circle) {
    if (!valid(box) || !valid(circle))
        return false;

    const auto bounds = worldBox(box);
    const Vec2f delta = circle.parent()->transform().transformPoint(circle.offset) - bounds.center;
    // 배율로 나누지 않고 회전된 박스의 좌표계에 투영한다.
    const Vec2f center{dot(delta, bounds.xAxis), dot(delta, bounds.yAxis)};
    const Vec2f closest{std::clamp(center.x, -bounds.halfSize.x, bounds.halfSize.x),
                        std::clamp(center.y, -bounds.halfSize.y, bounds.halfSize.y)};
    const float radius = worldRadius(circle);
    return (center - closest).lengthSquared() < radius * radius;
}

bool intersectsBoxBox(const BoxCollider& a, const BoxCollider& b) {
    if (!valid(a) || !valid(b))
        return false;

    const auto boundsA = worldBox(a);
    const auto boundsB = worldBox(b);
    const Vec2f delta = boundsB.center - boundsA.center;
    // 분리 축 정리: 각 사각형의 두 변에 수직인 축을 검사한다.
    for (const Vec2f axis : {boundsA.xAxis, boundsA.yAxis, boundsB.xAxis, boundsB.yAxis}) {
        const float extentA = boundsA.halfSize.x * std::abs(dot(boundsA.xAxis, axis)) +
                              boundsA.halfSize.y * std::abs(dot(boundsA.yAxis, axis));
        const float extentB = boundsB.halfSize.x * std::abs(dot(boundsB.xAxis, axis)) +
                              boundsB.halfSize.y * std::abs(dot(boundsB.yAxis, axis));
        if (!(std::abs(dot(delta, axis)) < extentA + extentB))
            return false;
    }
    return true;
}

bool intersectsCircleCircle(const CircleCollider& a, const CircleCollider& b) {
    if (!valid(a) || !valid(b))
        return false;

    const Vec2f centerA = a.parent()->transform().transformPoint(a.offset);
    const Vec2f centerB = b.parent()->transform().transformPoint(b.offset);
    const float radiusSum = worldRadius(a) + worldRadius(b);
    return (centerA - centerB).lengthSquared() < radiusSum * radiusSum;
}
} // namespace

bool BoxCollider::contains(Vec2f worldPoint) const {
    if (!valid(*this))
        return false;

    const auto bounds = worldBox(*this);
    const Vec2f delta = worldPoint - bounds.center;
    const float x = dot(delta, bounds.xAxis);
    const float y = dot(delta, bounds.yAxis);
    return -bounds.halfSize.x <= x && x < bounds.halfSize.x && -bounds.halfSize.y <= y &&
           y < bounds.halfSize.y;
}

bool intersects(const GameObject& a, const GameObject& b) {

    if (const auto* boxA = a.getComponent<BoxCollider>()) {

        if (const auto* boxB = b.getComponent<BoxCollider>()) {
            if (intersectsBoxBox(*boxA, *boxB))
                return true;
        }

        if (const auto* circleB = b.getComponent<CircleCollider>()) {
            if (intersectsBoxCircle(*boxA, *circleB))
                return true;
        }
    }

    if (const auto* circleA = a.getComponent<CircleCollider>()) {

        if (const auto* boxB = b.getComponent<BoxCollider>()) {
            if (intersectsBoxCircle(*boxB, *circleA))
                return true;
        }

        if (const auto* circleB = b.getComponent<CircleCollider>()) {
            if (intersectsCircleCircle(*circleA, *circleB))
                return true;
        }
    }

    return false;
}

} // namespace meno
