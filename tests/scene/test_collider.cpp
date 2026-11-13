#include <meno/scene/Collider.hpp>
#include <meno/scene/SceneManager.hpp>

#include "check.hpp"
#include <limits>

int main() {
    auto& manager = meno::SceneManager::instance();
    auto& scene = manager.createScene();
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

    // A NaN position must not report box-box overlap in either argument order.
    auto& invalidBox = scene.createGameObject();
    auto& distantBox = scene.createGameObject();
    invalidBox.addComponent<meno::BoxCollider>(meno::Vec2f{}, meno::Vec2f{10.f, 10.f});
    distantBox.addComponent<meno::BoxCollider>(meno::Vec2f{}, meno::Vec2f{10.f, 10.f});
    invalidBox.transform().pos = {std::numeric_limits<float>::quiet_NaN(), 0.f};
    distantBox.transform().pos = {1000.f, 1000.f};
    MENO_CHECK(!meno::intersects(invalidBox, distantBox));
    MENO_CHECK(!meno::intersects(distantBox, invalidBox));

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

    // Fractional scale applies to both offset and size, about the parent's top-left.
    auto& scaled = scene.createGameObject();
    auto& probe = scene.createGameObject();
    scaled.transform().pos = {100.f, 200.f};
    scaled.transform().magnitude = 0.5f;
    scaled.addComponent<meno::BoxCollider>(meno::Vec2f{10.f, 20.f}, meno::Vec2f{20.f, 10.f});
    probe.addComponent<meno::BoxCollider>(meno::Vec2f{}, meno::Vec2f{1.f, 1.f});
    probe.transform().pos = {114.f, 214.f};
    MENO_CHECK(meno::intersects(scaled, probe));
    probe.transform().pos = {115.f, 214.f};
    MENO_CHECK(!meno::intersects(scaled, probe));
    scaled.transform().magnitude = -0.5f; // Bounds [85,95] x [185,190].
    probe.transform().pos = {86.f, 186.f};
    MENO_CHECK(meno::intersects(scaled, probe));
    probe.transform().pos = {95.f, 186.f};
    MENO_CHECK(!meno::intersects(scaled, probe));

    // Rotation moves the offset and rotates the shape around the same parent origin.
    scaled.transform().magnitude = 0.5f;
    scaled.transform().rotation = 90;
    probe.transform().pos = {86.f, 206.f};
    MENO_CHECK(meno::intersects(scaled, probe));
    MENO_CHECK(meno::intersects(probe, scaled));
    probe.transform().pos = {106.f, 211.f};
    MENO_CHECK(!meno::intersects(scaled, probe));

    auto& round = scene.createGameObject();
    round.transform().pos = {100.f, 200.f};
    round.transform().magnitude = 0.5f;
    round.transform().rotation = 90;
    round.addComponent<meno::CircleCollider>(meno::Vec2f{12.f, 24.f}, 2.f);
    MENO_CHECK(meno::intersects(scaled, round)); // Center (88,206), radius 1.
    MENO_CHECK(meno::intersects(round, scaled));
    auto& roundProbe = scene.createGameObject();
    roundProbe.addComponent<meno::CircleCollider>(meno::Vec2f{}, 1.f);
    roundProbe.transform().pos = {89.5f, 206.f};
    MENO_CHECK(meno::intersects(round, roundProbe));
    roundProbe.transform().pos = {90.5f, 206.f};
    MENO_CHECK(!meno::intersects(round, roundProbe));
    round.transform().magnitude = -0.5f; // Center (112,194), radius stays positive.
    roundProbe.transform().pos = {113.f, 194.f};
    MENO_CHECK(meno::intersects(round, roundProbe));

    // A rotated thin rectangle's enclosing AABB must not cause false positives.
    auto& thin = scene.createGameObject();
    thin.transform().rotation = 45;
    thin.addComponent<meno::BoxCollider>(meno::Vec2f{}, meno::Vec2f{10.f, 1.f});
    probe.transform().pos = {0.f, 6.f};
    MENO_CHECK(!meno::intersects(thin, probe));
    MENO_CHECK(!meno::intersects(probe, thin));
    probe.transform().pos = {3.f, 3.f};
    MENO_CHECK(meno::intersects(thin, probe));
    roundProbe.transform().pos = {0.f, 6.f};
    MENO_CHECK(!meno::intersects(thin, roundProbe));
    roundProbe.transform().pos = {3.f, 3.f};
    MENO_CHECK(meno::intersects(thin, roundProbe));

    for (float scale : {0.f, std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN()}) {
        thin.transform().magnitude = scale;
        MENO_CHECK(!meno::intersects(thin, probe));
        MENO_CHECK(!meno::intersects(thin, roundProbe));
        roundProbe.transform().magnitude = scale;
        MENO_CHECK(!meno::intersects(roundProbe, round));
        MENO_CHECK(!meno::intersects(probe, roundProbe));
    }

    auto& pointTarget = scene.createGameObject();
    auto& pointBox = pointTarget.addComponent<meno::BoxCollider>(
        meno::Vec2f{2.f, 4.f}, meno::Vec2f{8.f, 4.f});
    pointTarget.transform().pos = {10.f, 20.f};
    MENO_CHECK(pointBox.contains({16.f, 26.f}));
    MENO_CHECK(!pointBox.contains({11.f, 26.f}));
    // Half-open bounds include the left/top edges and the top-left corner.
    for (const meno::Vec2f point : {meno::Vec2f{12.f, 26.f}, {16.f, 24.f}, {12.f, 24.f}})
        MENO_CHECK(pointBox.contains(point));
    for (const meno::Vec2f point : {meno::Vec2f{20.f, 26.f}, {16.f, 28.f},
                                  {20.f, 24.f}, {12.f, 28.f}, {20.f, 28.f}})
        MENO_CHECK(!pointBox.contains(point));

    pointTarget.transform().rotation = 90.f;
    pointTarget.transform().magnitude = 0.5f;
    MENO_CHECK(pointBox.contains({7.f, 23.f}));
    MENO_CHECK(!pointBox.contains({16.f, 26.f}));
    pointTarget.transform().magnitude = -0.5f;
    MENO_CHECK(pointBox.contains({13.f, 17.f}));
    MENO_CHECK(!pointBox.contains({7.f, 23.f}));

    // Inside the enclosing AABB but outside the rotated thin box.
    pointTarget.transform().pos = {};
    pointTarget.transform().rotation = 45.f;
    pointTarget.transform().magnitude = 1.f;
    pointBox.offset = {};
    pointBox.size = {10.f, 1.f};
    MENO_CHECK(!pointBox.contains({0.f, 6.f}));
    MENO_CHECK(pointBox.contains({3.f, 3.5f}));
    pointBox.isTrigger = true;
    pointBox.layer = 7;
    MENO_CHECK(pointBox.contains({3.f, 3.5f}));

    for (float scale : {0.f, std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN()}) {
        pointTarget.transform().magnitude = scale;
        MENO_CHECK(!pointBox.contains({3.f, 3.5f}));
    }
    pointTarget.transform().magnitude = 1.f;
    for (const meno::Vec2f size : {meno::Vec2f{0.f, 1.f}, {-1.f, 1.f},
                                 {10.f, 0.f}, {10.f, -1.f}}) {
        pointBox.size = size;
        MENO_CHECK(!pointBox.contains({3.f, 3.5f}));
    }
    pointBox.size = {10.f, 1.f};
    pointBox.parent = nullptr;
    MENO_CHECK(!pointBox.contains({3.f, 3.5f}));

    MENO_CHECK(manager.shutdown());
    return meno::test::report();
}
