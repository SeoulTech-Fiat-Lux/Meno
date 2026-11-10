#include <meno/scene/Scene.hpp>
#include <meno/scene/GameObject.hpp>
#include <meno/core/SceneManager.hpp>
#include <exception>

namespace meno {

[[nodiscard]]
GameObject* Scene::getGameObject(GameObject::GameObjectID id) noexcept {
    auto it = objects_.find(id);

    if (it != objects_.end()) return it->second.get();
    const auto pending = pending_objects_.find(id);
    return pending == pending_objects_.end() ? nullptr : pending->second.get();
}

[[nodiscard]]
const GameObject* Scene::getGameObject(GameObject::GameObjectID id) const noexcept {
    auto it = objects_.find(id);

    if (it != objects_.end()) return it->second.get();
    const auto pending = pending_objects_.find(id);
    return pending == pending_objects_.end() ? nullptr : pending->second.get();
}

[[nodiscard]]
bool Scene::deleteGameObject(GameObject::GameObjectID id) noexcept {
    auto* object = getGameObject(id);
    if (!object || object->pending_delete_) return false;
    if (updating_) {
        object->pending_delete_ = true;
        pending_deletions_ = true;
        return true;
    }
    // 객체를 파괴하기 전에 캐시의 포인터를 제거한다.
    draw_order_.clear();
    draw_order_dirty_ = true;
    objects_.erase(id);
    return true;
}

void Scene::applyObjectChanges() {
    if (!pending_deletions_ && pending_objects_.empty()) return;
    draw_order_.clear();
    draw_order_dirty_ = true;
    if (pending_deletions_) {
        const auto deleted = [](const auto& entry) {
            return entry.second->pending_delete_;
        };
        std::erase_if(objects_, deleted);
        std::erase_if(pending_objects_, deleted);
        pending_deletions_ = false;
    }
    objects_.merge(pending_objects_);
}

void Scene::onUpdate(double dt) {
    updating_ = true;
    std::exception_ptr error;
    try {
        if (update_handler_) update_handler_(dt);
    } catch (...) {
        error = std::current_exception();
    }
    updating_ = false;
    // 씬 전환/exit보다 먼저 반영하며, 콜백 예외 시에도 예약을 남기지 않는다.
    applyObjectChanges();
    if (error) std::rethrow_exception(error);
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
