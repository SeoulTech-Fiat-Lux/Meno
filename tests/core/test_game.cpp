#include <meno/core/Clock.hpp>
#include <meno/core/Game.hpp>
#include <meno/scene/Component.hpp>

#include "check.hpp"

#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace {

constexpr int skipReturnCode = 77;

bool displayTestsEnabled() {
    const char* value = std::getenv("MENO_DISPLAY_TESTS");
    return value != nullptr && *value != '\0' && std::string_view{value} != "0";
}

static_assert(std::is_base_of_v<meno::Application, meno::Game>);
static_assert(!std::is_copy_constructible_v<meno::Game>);
static_assert(!std::is_move_constructible_v<meno::Game>);
static_assert(std::is_nothrow_destructible_v<meno::Game>);

struct LifetimeProbe final : meno::Component {
    meno::Game& game;
    meno::ResourceManager* expectedResources;
    bool& destroyed;
    bool& dependenciesAlive;

    LifetimeProbe(meno::Game& owner, bool& wasDestroyed, bool& wereAlive)
        : game(owner), expectedResources(&owner.resources()), destroyed(wasDestroyed),
          dependenciesAlive(wereAlive) {}

    ~LifetimeProbe() override {
        // Scene은 Game 소멸자 본문에서 정리되므로 Window와 ResourceManager를
        // 아직 안전하게 사용할 수 있어야 한다.
        dependenciesAlive = game.window().isOpen() && &game.resources() == expectedResources;
        destroyed = true;
    }
};

class NormalGame final : public meno::Game {
public:
    NormalGame(bool& destroyed, bool& dependenciesAlive, bool& exited)
        : Game({.application = {.fixedTimeStep = 0.01},
                .windowSize = {160u, 90u},
                .title = "meno game lifecycle test"}),
          destroyed_(destroyed), dependenciesAlive_(dependenciesAlive), exited_(exited) {}

private:
    void onStart() override {
        auto& scene = scenes().createScene();
        scene.createGameObject().addComponent<LifetimeProbe>(*this, destroyed_, dependenciesAlive_);
        scene.setUpdateHandler([this](double) { stop(); });
        scene.setExitHandler([this] { exited_ = true; });
        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("failed to activate lifecycle scene");
        }
    }

    bool& destroyed_;
    bool& dependenciesAlive_;
    bool& exited_;
};

class ThrowingStartGame final : public meno::Game {
public:
    ThrowingStartGame(bool& destroyed, bool& dependenciesAlive)
        : Game({.windowSize = {160u, 90u}, .title = "meno game exception test"}),
          destroyed_(destroyed), dependenciesAlive_(dependenciesAlive) {}

private:
    void onStart() override {
        auto& scene = scenes().createScene();
        scene.createGameObject().addComponent<LifetimeProbe>(*this, destroyed_, dependenciesAlive_);
        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("failed to activate exception scene");
        }
        throw std::runtime_error("onStart failed");
    }

    bool& destroyed_;
    bool& dependenciesAlive_;
};

class ReentrantGame final : public meno::Game {
public:
    bool rejected = false;
    bool sceneSurvived = false;

private:
    void onStart() override {
        auto& scene = scenes().createScene();
        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("failed to activate reentry scene");
        }
        try {
            run();
        } catch (const std::logic_error&) {
            rejected = true;
        }
        sceneSurvived = scenes().activeScene() == &scene;
        stop();
    }
};

} // namespace

int main() {
    if (!displayTestsEnabled()) {
        return skipReturnCode;
    }

    using namespace std::chrono_literals;

    bool normalDestroyed = false;
    bool normalDependenciesAlive = false;
    bool exited = false;
    {
        NormalGame game(normalDestroyed, normalDependenciesAlive, exited);
        int calls = 0;
        meno::Clock clock(
            [&] { return meno::Clock::TimePoint{} + (calls++ == 0 ? 0ms : 20ms); },
            [](meno::Clock::Duration) {});
        game.run(clock);
        MENO_CHECK(exited);
        MENO_CHECK(normalDestroyed);
        MENO_CHECK(normalDependenciesAlive);
        MENO_CHECK(game.scenes().activeScene() == nullptr);
    }

    bool exceptionalDestroyed = false;
    bool exceptionalDependenciesAlive = false;
    bool caught = false;
    {
        ThrowingStartGame game(exceptionalDestroyed, exceptionalDependenciesAlive);
        try {
            game.run();
        } catch (const std::runtime_error&) {
            caught = true;
        }
    }
    MENO_CHECK(caught);
    MENO_CHECK(exceptionalDestroyed);
    MENO_CHECK(exceptionalDependenciesAlive);
    MENO_CHECK(meno::SceneManager::instance().activeScene() == nullptr);

    ReentrantGame reentrant;
    reentrant.run();
    MENO_CHECK(reentrant.rejected);
    MENO_CHECK(reentrant.sceneSurvived);
    MENO_CHECK(reentrant.scenes().activeScene() == nullptr);

    return meno::test::report();
}
