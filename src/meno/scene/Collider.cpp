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
    return collider.parent && std::isfinite(collider.parent->transform().magnitude)
        && collider.parent->transform().magnitude != 0.f;
}

bool valid(const BoxCollider& box) {
    return valid(static_cast<const Collider&>(box)) && box.size.x > 0.f && box.size.y > 0.f;
}

bool valid(const CircleCollider& circle) {
    return valid(static_cast<const Collider&>(circle)) && circle.radius > 0.f;
}

WorldBox worldBox(const BoxCollider& box) {
    const auto& transform = box.parent->transform();
    return {transform.transformPoint(box.offset + box.size * 0.5f),
            box.size * (std::abs(transform.magnitude) * 0.5f),
            transform.rotateVector({1.f, 0.f}), transform.rotateVector({0.f, 1.f})};
}

float worldRadius(const CircleCollider& circle) {
    return circle.radius * std::abs(circle.parent->transform().magnitude);
}

bool intersects_box_circle(const BoxCollider& box, const CircleCollider& circle) {
    if (!valid(box) || !valid(circle))
        return false;

    const auto bounds = worldBox(box);
    const Vec2f delta = circle.parent->transform().transformPoint(circle.offset) - bounds.center;
    // Project into the rotated box's frame without dividing by scale.
    const Vec2f center{dot(delta, bounds.xAxis), dot(delta, bounds.yAxis)};
    const Vec2f closest{
        std::clamp(center.x, -bounds.halfSize.x, bounds.halfSize.x),
        std::clamp(center.y, -bounds.halfSize.y, bounds.halfSize.y)
    };
    const float radius = worldRadius(circle);
    return (center - closest).lengthSquared() < radius * radius;
}

bool intersects_box_box(const BoxCollider& a, const BoxCollider& b) {
    if (!valid(a) || !valid(b))
        return false;

    const auto boundsA = worldBox(a);
    const auto boundsB = worldBox(b);
    const Vec2f delta = boundsB.center - boundsA.center;
    // Separating axis theorem: test the two edge normals of each rectangle.
    for (const Vec2f axis : {boundsA.xAxis, boundsA.yAxis, boundsB.xAxis, boundsB.yAxis}) {
        const float extentA = boundsA.halfSize.x * std::abs(dot(boundsA.xAxis, axis))
                            + boundsA.halfSize.y * std::abs(dot(boundsA.yAxis, axis));
        const float extentB = boundsB.halfSize.x * std::abs(dot(boundsB.xAxis, axis))
                            + boundsB.halfSize.y * std::abs(dot(boundsB.yAxis, axis));
        if (std::abs(dot(delta, axis)) >= extentA + extentB)
            return false;
    }
    return true;
}

bool intersects_circle_circle(const CircleCollider& a, const CircleCollider& b) {
    if (!valid(a) || !valid(b))
        return false;

    const Vec2f centerA = a.parent->transform().transformPoint(a.offset);
    const Vec2f centerB = b.parent->transform().transformPoint(b.offset);
    const float radiusSum = worldRadius(a) + worldRadius(b);
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
