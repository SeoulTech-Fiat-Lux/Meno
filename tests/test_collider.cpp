#include <meno/core/Scene.hpp>

#include "check.hpp"

int main() {
    meno::Scene scene;
    auto& a = scene.createGameObject();
    auto& b = scene.createGameObject();
    MENO_CHECK(!meno::intersects(a, b));

    a.transform().pos = {100.f, 200.f};
    auto& boxA = a.addComponent<meno::BoxCollider>(meno::Vec2f{10.f, 20.f}, meno::Vec2f{10.f, 10.f});
    MENO_CHECK(!meno::intersects(a, b));
    auto& boxB = b.addComponent<meno::BoxCollider>(meno::Vec2f{}, meno::Vec2f{2.f, 2.f});
    b.transform().pos = {112.f, 222.f};
    MENO_CHECK(meno::intersects(a, b)); // Containment with nonzero position and offset.
    b.transform().pos = {120.f, 222.f};
    MENO_CHECK(!meno::intersects(a, b)); // Edge contact.
    b.transform().pos = {120.f, 230.f};
    MENO_CHECK(!meno::intersects(a, b)); // Corner contact.
    b.transform().pos = {119.f, 229.f};
    MENO_CHECK(meno::intersects(a, b));
    boxB.size.x = 0.f;
    MENO_CHECK(!meno::intersects(a, b));
    boxB.size.x = -2.f;
    MENO_CHECK(!meno::intersects(a, b));

    auto& c = scene.createGameObject();
    c.transform().pos = {100.f, 200.f};
    auto& circleC = c.addComponent<meno::CircleCollider>(meno::Vec2f{15.f, 25.f}, 2.f);
    MENO_CHECK(meno::intersects(a, c)); // Circle inside box.
    MENO_CHECK(meno::intersects(c, a)); // Reverse dispatch.
    circleC.offset = {22.f, 25.f};
    MENO_CHECK(!meno::intersects(a, c)); // Tangent to side.
    circleC.offset = {21.f, 25.f};
    MENO_CHECK(meno::intersects(a, c));
    circleC.offset = {22.f, 32.f};
    MENO_CHECK(!meno::intersects(a, c)); // Near corner, outside circle radius.
    circleC.offset = {21.f, 31.f};
    MENO_CHECK(meno::intersects(a, c));

    auto& d = scene.createGameObject();
    auto& circleD = d.addComponent<meno::CircleCollider>(meno::Vec2f{}, 3.f);
    circleC.offset = {15.f, 25.f};
    d.transform().pos = {118.f, 229.f};
    MENO_CHECK(!meno::intersects(c, d)); // Distance 5, radii sum 5.
    d.transform().pos = {118.f, 228.f};
    MENO_CHECK(meno::intersects(c, d));
    MENO_CHECK(meno::intersects(d, c));
    d.transform().pos = {115.f, 225.f};
    MENO_CHECK(meno::intersects(c, d)); // Concentric circles.
    circleD.radius = 0.f;
    MENO_CHECK(!meno::intersects(c, d));
    circleD.radius = -1.f;
    MENO_CHECK(!meno::intersects(c, d));

    // A noncolliding box must not hide a colliding circle on the same object.
    b.addComponent<meno::CircleCollider>(meno::Vec2f{-4.f, -4.f}, 2.f);
    MENO_CHECK(meno::intersects(a, b));
    boxA.isTrigger = true;
    boxA.layer = 7;
    MENO_CHECK(meno::intersects(a, b));
    boxA.parent = nullptr;
    MENO_CHECK(!meno::intersects(a, b));

    return meno::test::report();
}
