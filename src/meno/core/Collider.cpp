#include <meno/core/Collider.hpp>

namespace meno {

namespace {

bool intersects_(const Collider& a, const Collider& b) {
        if (a.type() == ColliderType::Box && b.type() == ColliderType::Box) {
            // Box-Box intersection logic
        } else if (a.type() == ColliderType::Circle && b.type() == ColliderType::Circle) {
            // Circle-Circle intersection logic
        } else {
            // Mixed intersection logic
        }
        return false;
}

}

bool intersects(const GameObject& a, const GameObject& b) {
    return intersects_(a.collider(), b.collider());
}

} // namespace meno