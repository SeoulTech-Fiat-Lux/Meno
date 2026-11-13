#include <meno/scene/GameObject.hpp>
#include <meno/scene/Scene.hpp>
#include <meno/scene/SceneManager.hpp>
#include <exception>

namespace meno {

[[nodiscard]]
GameObject* Scene::getGameObject(GameObject::GameObjectID id) noexcept {
    auto it = objects_.find(id);

    if (it != objects_.end())
        return it->second.get();
    const auto pending = pendingObjects_.find(id);
    return pending == pendingObjects_.end() ? nullptr : pending->second.get();
}

[[nodiscard]]
const GameObject* Scene::getGameObject(GameObject::GameObjectID id) const noexcept {
    auto it = objects_.find(id);

    if (it != objects_.end())
        return it->second.get();
    const auto pending = pendingObjects_.find(id);
    return pending == pendingObjects_.end() ? nullptr : pending->second.get();
}

[[nodiscard]]
bool Scene::deleteGameObject(GameObject::GameObjectID id) noexcept {
    auto* object = getGameObject(id);
    if (!object || object->pendingDelete_)
        return false;
    if (updating_) {
        object->pendingDelete_ = true;
        pendingDeletions_ = true;
        return true;
    }
    // 객체를 파괴하기 전에 캐시의 포인터를 제거한다.
    drawOrder_.clear();
    drawOrderDirty_ = true;
    objects_.erase(id);
    return true;
}

void Scene::applyObjectChanges() {
    if (!pendingDeletions_ && pendingObjects_.empty())
        return;
    drawOrder_.clear();
    drawOrderDirty_ = true;
    if (pendingDeletions_) {
        const auto deleted = [](const auto& entry) { return entry.second->pendingDelete_; };
        std::erase_if(objects_, deleted);
        std::erase_if(pendingObjects_, deleted);
        pendingDeletions_ = false;
    }
    objects_.merge(pendingObjects_);
}

void Scene::onUpdate(double dt) {
    updating_ = true;
    std::exception_ptr error;
    try {
        if (updateHandler_)
            updateHandler_(dt);
    } catch (...) {
        error = std::current_exception();
    }
    updating_ = false;
    // 씬 전환/exit보다 먼저 반영하며, 콜백 예외 시에도 예약을 남기지 않는다.
    applyObjectChanges();
    if (error)
        std::rethrow_exception(error);
}

bool Scene::setLayer(GameObject::GameObjectID id, int layer) noexcept {
    auto* object = getGameObject(id);
    if (!object)
        return false;
    if (object->layer_ != layer) {
        object->layer_ = layer;
        drawOrderDirty_ = true;
    }
    return true;
}

} // namespace meno
