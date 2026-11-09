#include <meno/core/SceneManager.hpp>
#include <meno/core/Window.hpp>
#include "check.hpp"

#include <initializer_list>
#include <vector>

namespace {
std::vector<meno::GameObject::GameObjectID> drawn;
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

    MENO_CHECK(manager.shutdown());
    return meno::test::report();
}
