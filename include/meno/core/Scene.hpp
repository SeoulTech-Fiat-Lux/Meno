#ifndef MENO_SCENE
#define MENO_SCENE

#include <meno/core/GameObject.hpp>

#include <vector>
#include <memory>
#include <unordered_map>

namespace meno {

class Scene {

    std::unordered_map<GameObjectID, std::unique_ptr<GameObject>> objects_;
    GameObject::GameObjectID next_id_{1};

public:
    template <typename T = GameObject, typename... Args>
        requires std::derived_from<T, GameObject>
    T& createGameObject(Args&&... args) {
        auto object = std::make_unique<T>(next_id_, std::forward<Args>(args)...);

        T& ref = *object;
        objects_.emplace(next_id_++, std::move(object));

        return ref;
    }
};

} // namespace meno

#endif //MENO_SCENE