#include <meno/core/SceneManager.hpp>
#include <meno/core/Window.hpp>
#include "check.hpp"

#include <initializer_list>
#include <vector>
#include <stdexcept>

namespace {
std::vector<meno::GameObject::GameObjectID> drawn;
struct Lifetime : meno::Component {
    int& live;
    explicit Lifetime(int& count) : live(count) { ++live; }
    ~Lifetime() override { --live; }
};
}

// 실제 그래픽 구현은 링크하지 않는다. Sprite 호출 순서만 기록한다.
namespace meno {
struct Window::Impl {};
Window::Window(Vec2u, std::string_view) {}
Window::~Window() = default;
struct Renderer::Impl {};
Renderer::Renderer(Window&) {}
Renderer::~Renderer() = default;
void Sprite::draw(Renderer&) const { drawn.push_back(parent->id()); }
}

int main() {
    auto& manager = meno::SceneManager::instance();
    auto& scene = manager.createScene();
    meno::Window window({1, 1}, "draw order test");
    meno::Renderer renderer(window);
    const auto checkOrder = [&](std::initializer_list<meno::GameObject::GameObjectID> expected) {
        drawn.clear();
        const meno::Scene& constScene = scene;
        constScene.drawScene(renderer);
        MENO_CHECK(drawn == std::vector<meno::GameObject::GameObjectID>(expected));
    };

    checkOrder({});
    auto& first = scene.createGameObject();
    const auto a = first.id();
    const auto b = scene.createGameObject().id();
    const auto c = scene.createGameObject().id();
    MENO_CHECK(first.layer() == 0);
    checkOrder({a, b, c});
    MENO_CHECK(scene.setLayer(a, 10));
    MENO_CHECK(scene.setLayer(c, -5));
    MENO_CHECK(first.layer() == 10);
    checkOrder({c, b, a});
    checkOrder({c, b, a}); // 변경 없는 프레임
    MENO_CHECK(scene.setLayer(b, 10));
    checkOrder({c, a, b}); // 같은 레이어는 ID 순서
    MENO_CHECK(scene.setLayer(b, 10));
    MENO_CHECK(!scene.setLayer(999, 1));
    checkOrder({c, a, b});

    const auto d = scene.createGameObject().id();
    checkOrder({c, d, a, b}); // 캐시 생성 이후 객체 추가
    MENO_CHECK(scene.deleteGameObject(c));
    MENO_CHECK(!scene.deleteGameObject(c));
    MENO_CHECK(!scene.setLayer(c, 0));
    checkOrder({d, a, b}); // 삭제된 객체를 그리지 않는다
    MENO_CHECK(scene.setLayer(b, -10));
    checkOrder({b, d, a});
    MENO_CHECK(scene.deleteGameObject(a));
    MENO_CHECK(scene.deleteGameObject(b));
    MENO_CHECK(scene.deleteGameObject(d));
    checkOrder({});
    const auto e = scene.createGameObject().id();
    checkOrder({e});

    // 업데이트 중 생성/삭제가 기존 그리기 목록과 참조를 바꾸지 않는다.
    int live = 0;
    scene.getGameObject(e)->addComponent<Lifetime>(live);
    meno::GameObject::GameObjectID added = 0, cancelled = 0;
    meno::GameObject* addedPointer = nullptr;
    MENO_CHECK(manager.changeScene(scene.id()));
    scene.setUpdateHandler([&](double dt) {
        auto* existing = scene.getGameObject(e);
        MENO_CHECK(scene.deleteGameObject(e));
        MENO_CHECK(!scene.deleteGameObject(e));
        MENO_CHECK(!scene.deleteGameObject(999999));
        MENO_CHECK(live == 1);
        MENO_CHECK(scene.getGameObject(e) == existing);
        existing->transform().rotation = 1.5f;
        auto& created = scene.createGameObject();
        added = created.id();
        addedPointer = &created;
        created.addComponent<Lifetime>(live);
        MENO_CHECK(scene.getGameObject(added) == &created);
        MENO_CHECK(static_cast<const meno::Scene&>(scene).getGameObject(added) == &created);
        MENO_CHECK(scene.setLayer(added, -5));
        auto& transient = scene.createGameObject();
        cancelled = transient.id();
        transient.addComponent<Lifetime>(live);
        MENO_CHECK(scene.deleteGameObject(cancelled));
        MENO_CHECK(!scene.deleteGameObject(cancelled));
        MENO_CHECK(live == 3);
        // 다수 생성으로 예약 컨테이너의 재해시를 유발한다.
        for (int i = 0; i < 1000; ++i) {
            const auto id = scene.createGameObject().id();
            MENO_CHECK(scene.deleteGameObject(id));
        }
        checkOrder({e});
        manager.update(dt); // 재진입이 예약을 조기 반영하지 않는다.
        MENO_CHECK(live == 3);
        checkOrder({e});
    });
    manager.update(0.1);
    MENO_CHECK(live == 1);
    MENO_CHECK(scene.getGameObject(e) == nullptr);
    MENO_CHECK(scene.getGameObject(cancelled) == nullptr);
    MENO_CHECK(scene.getGameObject(added) == addedPointer);
    MENO_CHECK(addedPointer->layer() == -5);
    checkOrder({added});

    // 예외 경로에서도 수명/예약을 정리하고 다음 업데이트를 허용한다.
    meno::GameObject::GameObjectID afterException = 0;
    scene.setUpdateHandler([&](double) {
        MENO_CHECK(scene.deleteGameObject(added));
        afterException = scene.createGameObject().id();
        throw std::runtime_error("update");
    });
    bool threw = false;
    try { manager.update(0.1); }
    catch (const std::runtime_error&) { threw = true; }
    MENO_CHECK(threw);
    MENO_CHECK(live == 0);
    MENO_CHECK(scene.getGameObject(added) == nullptr);
    checkOrder({afterException});
    scene.setUpdateHandler({});
    manager.update(0.1);
    // 업데이트 밖의 생성/삭제는 즉시 반영한다.
    const auto immediate = scene.createGameObject().id();
    checkOrder({afterException, immediate});
    MENO_CHECK(scene.deleteGameObject(immediate));
    checkOrder({afterException});

    // 객체 변경은 씬 전환의 exit/enter보다 먼저 반영한다.
    auto& next = manager.createScene();
    meno::GameObject::GameObjectID beforeExit = 0;
    scene.setUpdateHandler([&](double) {
        MENO_CHECK(scene.deleteGameObject(afterException));
        beforeExit = scene.createGameObject().id();
        MENO_CHECK(manager.changeScene(next.id()));
    });
    scene.setExitHandler([&] {
        MENO_CHECK(scene.getGameObject(afterException) == nullptr);
        MENO_CHECK(scene.getGameObject(beforeExit) != nullptr);
        checkOrder({beforeExit});
    });
    next.setEnterHandler([&] { checkOrder({beforeExit}); });
    manager.update(0.1);
    MENO_CHECK(manager.activeScene() == &next);

    MENO_CHECK(manager.shutdown());
    return meno::test::report();
}
