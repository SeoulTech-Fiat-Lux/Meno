#include <meno/scene/Scene.hpp>
#include <meno/scene/GameObject.hpp>
#include <meno/core/SceneManager.hpp>

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
    const auto it = objects_.find(id);
    if (it == objects_.end()) return false;
    // 객체를 파괴하기 전에 캐시의 포인터를 제거한다.
    draw_order_.clear();
    draw_order_dirty_ = true;
    objects_.erase(it);
    return true;
}

bool Scene::setLayer(GameObject::GameObjectID id, int layer) noexcept {
    auto* object = getGameObject(id);
    if (!object) return false;
    if (object->layer_ != layer) {
        object->layer_ = layer;
        draw_order_dirty_ = true;
    }
    return true;
}

}
