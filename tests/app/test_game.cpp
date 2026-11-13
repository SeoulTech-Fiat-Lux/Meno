#include <meno/app/Game.hpp>
#include <meno/core/Clock.hpp>
#include <meno/scene/Component.hpp>

#include "check.hpp"
#include "display.hpp"

#include <chrono>
#include <stdexcept>
#include <type_traits>

namespace {
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

class ExitReentrantGame final : public meno::Game {
public:
    bool rejected = false;
    bool clockOverloadRejected = false;

private:
    void onStart() override {
        auto& scene = scenes().createScene();
        scene.setUpdateHandler([this](double) { stop(); });
        scene.setExitHandler([this] {
            try {
                run();
            } catch (const std::logic_error&) {
                rejected = true;
            }
            try {
                meno::Clock clock;
                run(clock);
            } catch (const std::logic_error&) {
                clockOverloadRejected = true;
            }
        });
        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("failed to activate exit reentry scene");
        }
    }
};

} // namespace

int main() {
    if (meno::test::skipWithoutDisplay()) {
        return meno::test::kSkipReturnCode;
    }

    using namespace std::chrono_literals;

    bool normalDestroyed = false;
    bool normalDependenciesAlive = false;
    bool exited = false;
    {
        NormalGame game(normalDestroyed, normalDependenciesAlive, exited);
        int calls = 0;
        meno::Clock clock([&] { return meno::Clock::TimePoint{} + (calls++ == 0 ? 0ms : 20ms); },
                          [](meno::Clock::Duration) {});
        // Application 참조로 실행해도 가상 dispatch를 통해 Game의 정리 경로를 탄다.
        meno::Application& application = game;
        application.run(clock);
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
            meno::Application& application = game;
            application.run();
        } catch (const std::runtime_error&) {
            caught = true;
        }
        MENO_CHECK(game.scenes().activeScene() == nullptr);
    }
    MENO_CHECK(caught);
    MENO_CHECK(exceptionalDestroyed);
    MENO_CHECK(exceptionalDependenciesAlive);

    {
        ReentrantGame reentrant;
        reentrant.run();
        MENO_CHECK(reentrant.rejected);
        MENO_CHECK(reentrant.sceneSurvived);
        MENO_CHECK(reentrant.scenes().activeScene() == nullptr);
    }

    {
        ExitReentrantGame exitReentrant;
        exitReentrant.run();
        MENO_CHECK(exitReentrant.rejected);
        MENO_CHECK(exitReentrant.clockOverloadRejected);
    }

    {
        meno::Game first({.windowSize = {160u, 90u}, .title = "meno first game"});
        auto& firstScene = first.scenes().createScene();
        MENO_CHECK(first.scenes().changeScene(firstScene.id()));
        bool secondDestroyed = false;
        bool secondDependenciesAlive = false;
        {
            meno::Game second({.windowSize = {160u, 90u}, .title = "meno second game"});
            auto& secondScene = second.scenes().createScene();
            secondScene.createGameObject().addComponent<LifetimeProbe>(second, secondDestroyed,
                                                                       secondDependenciesAlive);
            MENO_CHECK(second.scenes().changeScene(secondScene.id()));
            MENO_CHECK(&first.scenes() != &second.scenes());
            MENO_CHECK(first.scenes().activeScene() == &firstScene);
            MENO_CHECK(second.scenes().activeScene() == &secondScene);
        }
        MENO_CHECK(secondDestroyed);
        MENO_CHECK(secondDependenciesAlive);
        MENO_CHECK(first.scenes().activeScene() == &firstScene);
        MENO_CHECK(first.scenes().getScene(firstScene.id()) == &firstScene);
    }

    bool discarded = false;
    bool discardDependenciesAlive = false;
    bool exitCalledFromDestructor = false;
    {
        meno::Game game({.windowSize = {160u, 90u}, .title = "meno game discard test"});
        auto& scene = game.scenes().createScene();
        scene.createGameObject().addComponent<LifetimeProbe>(game, discarded,
                                                             discardDependenciesAlive);
        scene.setExitHandler([&] { exitCalledFromDestructor = true; });
        MENO_CHECK(game.scenes().changeScene(scene.id()));
        // run()하지 않은 Game도 소유 Scene을 폐기해야 하지만, 이 시점의
        // 파생 멤버는 이미 파괴됐을 수 있으므로 onExit은 호출하지 않는다.
    }
    MENO_CHECK(discarded);
    MENO_CHECK(discardDependenciesAlive);
    MENO_CHECK(!exitCalledFromDestructor);

    return meno::test::report();
}
