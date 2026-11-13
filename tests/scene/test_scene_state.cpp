#include <meno/scene/SceneManager.hpp>

#include "check.hpp"

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {
struct State {
    int& live;

    explicit State(int& count) : live(count) { ++live; }
    ~State() { --live; }

    State(const State&) = delete;
    State& operator=(const State&) = delete;
};

static_assert(std::is_same_v<decltype(std::declval<meno::Scene&>().getState<State>()), State*>);
static_assert(
    std::is_same_v<decltype(std::declval<const meno::Scene&>().getState<State>()), const State*>);

void testRegistration(meno::SceneManager& manager) {
    int originalLive = 0;
    int rejectedLive = 0;
    auto& scene = manager.createScene();
    const meno::Scene& constScene = scene;

    MENO_CHECK(scene.getState<State>() == nullptr);
    MENO_CHECK(constScene.getState<State>() == nullptr);

    auto original = std::make_unique<State>(originalLive);
    auto* originalPtr = original.get();
    scene.setState(std::move(original));
    MENO_CHECK(originalLive == 1);
    MENO_CHECK(scene.getState<State>() == originalPtr);
    MENO_CHECK(constScene.getState<State>() == originalPtr);

    auto rejected = std::make_unique<State>(rejectedLive);
    MENO_CHECK(rejectedLive == 1);
    bool threw = false;
    try {
        scene.setState(std::move(rejected));
    } catch (const std::logic_error&) {
        threw = true;
    }
    MENO_CHECK(threw);
    MENO_CHECK(originalLive == 1);
    MENO_CHECK(rejectedLive == 0);
    MENO_CHECK(scene.getState<State>() == originalPtr);
    MENO_CHECK(constScene.getState<State>() == originalPtr);

    MENO_CHECK(manager.shutdown());
    MENO_CHECK(originalLive == 0);
    MENO_CHECK(rejectedLive == 0);
}

void testEmptyRegistration(meno::SceneManager& manager) {
    int live = 0;
    auto& scene = manager.createScene();
    const meno::Scene& constScene = scene;
    scene.setState(std::unique_ptr<State>{});
    MENO_CHECK(scene.getState<State>() == nullptr);
    MENO_CHECK(constScene.getState<State>() == nullptr);

    auto state = std::make_unique<State>(live);
    auto* statePtr = state.get();
    scene.setState(std::move(state));
    MENO_CHECK(live == 1);
    MENO_CHECK(scene.getState<State>() == statePtr);
    MENO_CHECK(constScene.getState<State>() == statePtr);
    MENO_CHECK(manager.shutdown());
    MENO_CHECK(live == 0);
}

void testLifetime(meno::SceneManager& manager) {
    int firstLive = 0;
    int secondLive = 0;
    int inactiveLive = 0;
    auto& first = manager.createScene();
    auto& second = manager.createScene();
    auto& inactive = manager.createScene();
    const auto firstId = first.id();
    const auto secondId = second.id();
    first.setState(std::make_unique<State>(firstLive));
    second.setState(std::make_unique<State>(secondLive));
    inactive.setState(std::make_unique<State>(inactiveLive));
    auto* firstPtr = first.getState<State>();
    auto* secondPtr = second.getState<State>();

    MENO_CHECK(manager.changeScene(firstId));
    MENO_CHECK(manager.changeScene(secondId));
    MENO_CHECK(firstLive == 1);
    MENO_CHECK(secondLive == 1);
    MENO_CHECK(first.getState<State>() == firstPtr);
    MENO_CHECK(second.getState<State>() == secondPtr);

    MENO_CHECK(manager.deleteScene(firstId));
    MENO_CHECK(firstLive == 0);
    MENO_CHECK(secondLive == 1);
    MENO_CHECK(inactiveLive == 1);

    // shutdown은 활성 Scene과 비활성 Scene의 상태를 모두 파괴한다.
    MENO_CHECK(manager.shutdown());
    MENO_CHECK(firstLive == 0);
    MENO_CHECK(secondLive == 0);
    MENO_CHECK(inactiveLive == 0);
}
} // namespace

int main() {
    meno::SceneManager manager;
    testRegistration(manager);
    testEmptyRegistration(manager);
    testLifetime(manager);
    return meno::test::report();
}
