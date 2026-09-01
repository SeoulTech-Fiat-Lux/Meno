#include <meno/core/Collider.hpp>
#include <meno/core/GameObject.hpp>

#include <typeinfo>

namespace meno {

namespace {

bool intersects_box_circle(const meno::BoxCollider& box, const meno::CircleCollider& circle) {
    // ...
    return false;
}

bool intersects_box_box(const meno::BoxCollider& a, const meno::BoxCollider& b) {
    // ...
    return false;
}

bool intersects_circle_circle(const meno::CircleCollider& a, const meno::CircleCollider& b) {
    // ...
    return false;
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