#include <meno/scene/SceneManager.hpp>

#include <exception>
#include <utility>

namespace meno {
namespace {
// 핸들러 안에서 draw()가 호출되어도 바깥 핸들러의 busy 상태를 지우지 않도록 이전 값으로
// 되돌린다.
struct CallbackGuard {
    bool& busy;
    bool previous;
    explicit CallbackGuard(bool& flag) : busy(flag), previous(flag) { busy = true; }
    CallbackGuard(const CallbackGuard&) = delete;
    CallbackGuard& operator=(const CallbackGuard&) = delete;
    ~CallbackGuard() { busy = previous; }
};
} // namespace

Scene& SceneManager::createScene() {
    const auto id = nextId_++;
    std::unique_ptr<Scene, SceneDeleter> scene{new Scene(id)};
    auto& result = *scene;
    scenes_.emplace(id, std::move(scene));
    return result;
}

const Scene* SceneManager::getScene(Scene::SceneID id) const noexcept {
    const auto it = scenes_.find(id);
    return it == scenes_.end() ? nullptr : it->second.get();
}

Scene* SceneManager::getScene(Scene::SceneID id) noexcept {
    // 구현은 const 쪽 하나만 둔다. *this가 non-const이므로 const를 벗겨도 안전하다.
    return const_cast<Scene*>(std::as_const(*this).getScene(id));
}

bool SceneManager::changeScene(Scene::SceneID id) {
    if (!getScene(id))
        return false;
    pending_ = id;
    if (!busy_)
        applyPending();
    return true;
}

void SceneManager::applyPending() {
    if (!pending_)
        return;
    auto* next = getScene(*pending_);
    pending_.reset();
    if (!next || next == active_)
        return;
    CallbackGuard guard{busy_};
    // exit이 예외를 던지면 기존 Scene을 유지한다.
    if (active_)
        active_->onExit();
    active_ = next;
    // enter가 예외를 던져도 새 Scene의 소유권과 활성 상태는 유지한다.
    active_->onEnter();
}

void SceneManager::discard() noexcept {
    active_ = nullptr;
    pending_.reset();
    scenes_.clear();
    busy_ = false;
}

bool SceneManager::deleteScene(Scene::SceneID id) {
    if (busy_ || (active_ && active_->id() == id))
        return false;
    if (pending_ == id)
        pending_.reset();
    return scenes_.erase(id) != 0;
}

void SceneManager::update(double dt) {
    if (busy_)
        return;
    applyPending();
    {
        CallbackGuard guard{busy_};
        if (active_)
            active_->onUpdate(dt);
    }
    applyPending();
}

void SceneManager::draw(Renderer& renderer) {
    if (!active_)
        return;
    // draw 핸들러가 전환이나 삭제를 요청해도 그리는 중인 Scene이 바뀌거나 파괴되지 않게 한다.
    CallbackGuard guard{busy_};
    active_->drawScene(renderer);
}

bool SceneManager::shutdown() {
    if (busy_)
        return false;
    CallbackGuard guard{busy_};
    std::exception_ptr exitError;
    try {
        if (active_)
            active_->onExit();
    } catch (...) {
        exitError = std::current_exception();
    }
    active_ = nullptr;
    pending_.reset();
    scenes_.clear();
    if (exitError)
        std::rethrow_exception(exitError);
    return true;
}

} // namespace meno
