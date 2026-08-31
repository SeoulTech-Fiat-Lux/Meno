#include <meno/core/Collider.hpp>
#include <meno/core/GameObject.hpp>

#include <typeinfo>

namespace meno {

namespace {

template <typename A, typename B>
    requires std::derived_from<A, meno::Collider> && 
        std::derived_from<B, meno::Collider>
bool intersects_(const A& a, const B& b) {
    if constexpr (std::is_same_v<A, BoxCollider> && std::is_same_v<B, BoxCollider>) {
        // Box-Box
    } else if constexpr (std::is_same_v<A, CircleCollider> && std::is_same_v<B, CircleCollider>) {
        // Circle-Circle
    } else {
        // Box-Circle
    }
}

}

bool intersects(const GameObject& a, const GameObject& b) {
    return intersects_(a.collider(), b.collider());
}

} // namespace meno