#include <meno/core/Scene.hpp>
#include <meno/core/GameObject.hpp>

namespace meno {

[[nodiscard]]
GameObject* Scene::getGameObject(GameObject::GameObjectID id) noexcept {
    auto it = objects_.find(id);

    if (it == objects_.end())
        return nullptr;

    return it->second.get();
}

[[nodiscard]]
const GameObject* Scene::getGameObject(GameObject::GameObjectID id) const noexcept {
    auto it = objects_.find(id);

    if (it == objects_.end())
        return nullptr;

    return it->second.get();
}

[[nodiscard]]
bool Scene::deleteGameObject(GameObject::GameObjectID id) noexcept {
    return objects_.erase(id) != 0;
}

}
