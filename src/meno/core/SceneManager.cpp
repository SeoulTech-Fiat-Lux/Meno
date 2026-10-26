#include <meno/core/SceneManager.hpp>
#include <exception>
#include <limits>
#include <stdexcept>

namespace meno {
namespace {
struct CallbackGuard {
    bool& busy;
    explicit CallbackGuard(bool& flag) : busy(flag) { busy = true; }
    ~CallbackGuard() { busy = false; }
};
}

SceneManager& SceneManager::instance() {
    static SceneManager manager;
    return manager;
}

Scene& SceneManager::createScene() {
    if (nextId_ == std::numeric_limits<Scene::SceneID>::max())
        throw std::overflow_error("Scene ID exhausted");
    const auto id = nextId_++;
    std::unique_ptr<Scene, SceneDeleter> scene{new Scene(id)};
    auto& result = *scene;
    scenes_.emplace(id, std::move(scene));
    return result;
}

Scene* SceneManager::getScene(Scene::SceneID id) noexcept {
    const auto it = scenes_.find(id);
    return it == scenes_.end() ? nullptr : it->second.get();
}

const Scene* SceneManager::getScene(Scene::SceneID id) const noexcept {
    const auto it = scenes_.find(id);
    return it == scenes_.end() ? nullptr : it->second.get();
}

bool SceneManager::changeScene(Scene::SceneID id) {
    if (!getScene(id)) return false;
    pending_ = id;
    if (!busy_) applyPending();
    return true;
}

void SceneManager::applyPending() {
    if (!pending_) return;
    auto* next = getScene(*pending_);
    pending_.reset();
    if (!next || next == active_) return;
    CallbackGuard guard{busy_};
    // exit이 예외를 던지면 기존 Scene을 유지한다.
    if (active_) active_->onExit();
    active_ = next;
    // enter가 예외를 던져도 새 Scene의 소유권과 활성 상태는 유지한다.
    active_->onEnter();
}

bool SceneManager::deleteScene(Scene::SceneID id) {
    if (busy_ || (active_ && active_->id() == id)) return false;
    if (pending_ == id) pending_.reset();
    return scenes_.erase(id) != 0;
}

void SceneManager::update(double dt) {
    if (busy_) return;
    applyPending();
    {
        CallbackGuard guard{busy_};
        if (active_) active_->onUpdate(dt);
    }
    applyPending();
}

bool SceneManager::shutdown() {
    if (busy_) return false;
    CallbackGuard guard{busy_};
    std::exception_ptr exitError;
    try {
        if (active_) active_->onExit();
    } catch (...) {
        exitError = std::current_exception();
    }
    active_ = nullptr;
    pending_.reset();
    scenes_.clear();
    if (exitError) std::rethrow_exception(exitError);
    return true;
}

} // namespace meno
