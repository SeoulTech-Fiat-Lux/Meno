#include "check.hpp"
#include <meno/scene/SceneManager.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>

static_assert(!std::is_default_constructible_v<meno::Scene>);
static_assert(!std::is_destructible_v<meno::Scene>);
static_assert(!std::is_move_constructible_v<meno::Scene>);
static_assert(std::is_default_constructible_v<meno::SceneManager>);
static_assert(std::is_destructible_v<meno::SceneManager>);
static_assert(!std::is_copy_constructible_v<meno::SceneManager>);
static_assert(!std::is_move_constructible_v<meno::SceneManager>);

namespace {
struct Lifetime final : meno::Component {
    int& count;
    explicit Lifetime(int& live) : count(live) { ++count; }
    ~Lifetime() override { --count; }
};
} // namespace

int main() {
    meno::SceneManager manager;
    MENO_CHECK(manager.activeScene() == nullptr);
    manager.update(0.1f);
    int live = 0;
    auto& first = manager.createScene();
    auto& second = manager.createScene();
    first.createGameObject().addComponent<Lifetime>(live);
    second.createGameObject().addComponent<Lifetime>(live);
    MENO_CHECK(live == 2);
    const auto firstId = first.id();
    const auto secondId = second.id();
    MENO_CHECK(firstId != secondId);
    MENO_CHECK(manager.getScene(firstId) == &first);
    const auto& constManager = manager;
    MENO_CHECK(constManager.getScene(secondId) == &second);
    MENO_CHECK(!manager.changeScene(0));
    MENO_CHECK(!manager.deleteScene(0));
    std::string events;
    first.setEnterHandler([&] { events += 'A'; });
    first.setExitHandler([&] { events += 'a'; });
    second.setEnterHandler([&] { events += 'B'; });
    second.setExitHandler([&] { events += 'b'; });
    first.setUpdateHandler([&](double dt) {
        MENO_CHECK(dt == 0.25f);
        events += 'U';
        MENO_CHECK(manager.changeScene(secondId));
        MENO_CHECK(!manager.changeScene(0));
        MENO_CHECK(manager.activeScene() == &first);
        MENO_CHECK(!manager.deleteScene(secondId));
        MENO_CHECK(!manager.shutdown());
        manager.update(dt); // 재진입은 무시한다.
        events += 'u';
    });
    MENO_CHECK(manager.changeScene(firstId));
    MENO_CHECK(manager.changeScene(firstId));
    MENO_CHECK(events == "A");
    MENO_CHECK(!manager.deleteScene(firstId));
    manager.update(0.25f);
    MENO_CHECK(events == "AUuaB");
    MENO_CHECK(constManager.activeScene() == &second);
    MENO_CHECK(manager.deleteScene(firstId));
    MENO_CHECK(manager.getScene(firstId) == nullptr);
    MENO_CHECK(live == 1);
    second.setUpdateHandler([](double) { throw std::runtime_error("test"); });
    try {
        manager.update(0.1f);
        MENO_CHECK(false);
    } catch (const std::runtime_error&) {
    }
    MENO_CHECK(manager.shutdown());
    MENO_CHECK(events == "AUuaBb");
    MENO_CHECK(live == 0);
    MENO_CHECK(manager.shutdown());
    MENO_CHECK(manager.activeScene() == nullptr);
    MENO_CHECK(manager.getScene(secondId) == nullptr);

    auto& third = manager.createScene();
    auto& fourth = manager.createScene();
    MENO_CHECK(third.id() > secondId);
    third.setEnterHandler([&] { MENO_CHECK(manager.changeScene(fourth.id())); });
    MENO_CHECK(manager.changeScene(third.id()));
    MENO_CHECK(manager.activeScene() == &third);
    manager.update(0.1f);
    MENO_CHECK(manager.activeScene() == &fourth);
    fourth.setExitHandler([] { throw std::runtime_error("exit"); });
    try {
        (void)manager.changeScene(third.id());
        MENO_CHECK(false);
    } catch (const std::runtime_error&) {
    }
    MENO_CHECK(manager.activeScene() == &fourth);
    fourth.setExitHandler({});
    third.setEnterHandler([] { throw std::runtime_error("enter"); });
    try {
        (void)manager.changeScene(third.id());
        MENO_CHECK(false);
    } catch (const std::runtime_error&) {
    }
    MENO_CHECK(manager.activeScene() == &third);
    MENO_CHECK(manager.shutdown());
    auto& cleanupScene = manager.createScene();
    auto& pendingScene = manager.createScene();
    const auto cleanupId = cleanupScene.id();
    const auto pendingId = pendingScene.id();
    cleanupScene.createGameObject().addComponent<Lifetime>(live);
    pendingScene.createGameObject().addComponent<Lifetime>(live);
    MENO_CHECK(manager.changeScene(cleanupId));
    constexpr double preciseDelta = 0.123456789012345;
    double observedDelta = 0.0;
    cleanupScene.setUpdateHandler([&](double dt) { observedDelta = dt; });
    manager.update(preciseDelta);
    MENO_CHECK(observedDelta == preciseDelta);
    cleanupScene.setExitHandler([&] {
        MENO_CHECK(manager.changeScene(pendingId));
        throw std::runtime_error("shutdown exit");
    });
    bool shutdownThrew = false;
    try {
        (void)manager.shutdown();
    } catch (const std::runtime_error& error) {
        shutdownThrew = true;
        MENO_CHECK(std::string(error.what()) == "shutdown exit");
    }
    MENO_CHECK(shutdownThrew);
    MENO_CHECK(live == 0);
    MENO_CHECK(manager.activeScene() == nullptr);
    MENO_CHECK(manager.getScene(cleanupId) == nullptr);
    MENO_CHECK(manager.getScene(pendingId) == nullptr);
    manager.update(preciseDelta);
    MENO_CHECK(manager.activeScene() == nullptr);
    auto& restartedScene = manager.createScene();
    MENO_CHECK(manager.changeScene(restartedScene.id()));
    MENO_CHECK(manager.activeScene() == &restartedScene);
    MENO_CHECK(manager.shutdown());

    bool exitCalledFromDestructor = false;
    {
        meno::SceneManager ownedManager;
        auto& ownedScene = ownedManager.createScene();
        ownedScene.createGameObject().addComponent<Lifetime>(live);
        ownedScene.setExitHandler([&] { exitCalledFromDestructor = true; });
        MENO_CHECK(ownedManager.changeScene(ownedScene.id()));
        MENO_CHECK(manager.activeScene() == nullptr);
        MENO_CHECK(manager.getScene(ownedScene.id()) == nullptr);
        MENO_CHECK(live == 1);
    }
    MENO_CHECK(live == 0);
    MENO_CHECK(!exitCalledFromDestructor);
    return meno::test::report();
}
