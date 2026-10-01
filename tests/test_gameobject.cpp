#include <meno/core/Component.hpp>
#include <meno/core/Sprite.hpp>
#include <meno/core/GameObject.hpp>
#include <meno/core/Scene.hpp>

#include "check.hpp"

#include <type_traits>
#include <cmath>

namespace {

struct Health final : meno::Component {
    explicit Health(int initial) : value(initial) {}

    int value;
};

struct Name final : meno::Component {
    explicit Name(const char* initial) : value(initial) {}

    const char* value;
};

template <typename T>
concept CanAddComponent = requires(meno::GameObject& object) {
    object.template addComponent<T>();
};

} // namespace

int main() {
    meno::Component detached;
    MENO_CHECK(detached.parent == nullptr);

    static_assert(!std::is_copy_constructible_v<meno::GameObject>);
    static_assert(!std::is_copy_assignable_v<meno::GameObject>);

    static_assert(CanAddComponent<Health>);
    static_assert(!CanAddComponent<meno::Transform>);
    static_assert(!CanAddComponent<meno::Sprite>);

    meno::Scene scene;
    meno::GameObject& object = scene.createGameObject();
    MENO_CHECK(object.transform().parent == &object);
    meno::Sprite sprite(&object);
    MENO_CHECK(sprite.parent == &object);
    MENO_CHECK(object.sprite().parent == &object);
    MENO_CHECK(!object.sprite().texture());
    meno::Sprite detachedSprite(nullptr);
    MENO_CHECK(detachedSprite.drawParams().position == meno::Vec2f{0.f, 0.f});
    MENO_CHECK(detachedSprite.drawParams().scale == meno::Vec2f{1.f, 1.f});

    MENO_CHECK(object.id() == 1);
    MENO_CHECK(object.transform().pos == meno::Vec2f{0.f, 0.f});
    MENO_CHECK(object.transform().magnitude == 1);
    MENO_CHECK(object.transform().rotation == 0);
    MENO_CHECK(object.transform().width == 0);
    MENO_CHECK(object.transform().height == 0);

    object.transform().pos = meno::Vec2f{12.f, 34.f};
    object.transform().magnitude = 2;
    object.transform().rotation = 90;
    object.transform().width = 64;
    object.transform().height = 32;

    MENO_CHECK(object.transform().pos == meno::Vec2f{12.f, 34.f});
    MENO_CHECK(object.transform().magnitude == 2);
    MENO_CHECK(object.transform().rotation == 90);
    MENO_CHECK(object.transform().width == 64);
    MENO_CHECK(object.transform().height == 32);

    object.sprite().origin = {8.f, 16.f};
    object.sprite().tint = meno::colors::Red;
    object.sprite().source = meno::Recti{{16, 32}, {16, 16}};
    object.sprite().flipX = true;
    object.sprite().flipY = true;
    const auto params = object.sprite().drawParams();
    MENO_CHECK(params.position == meno::Vec2f{12.f, 34.f});
    MENO_CHECK(params.scale == meno::Vec2f{2.f, 2.f});
    MENO_CHECK(params.rotation == 90.f);
    MENO_CHECK(params.origin == meno::Vec2f{8.f, 16.f});
    MENO_CHECK(params.tint == meno::colors::Red);
    MENO_CHECK(params.source.has_value());
    MENO_CHECK(params.source->position == meno::Vec2i{16, 32});
    MENO_CHECK(params.source->size == meno::Vec2i{16, 16});
    MENO_CHECK(params.flipX && params.flipY);

    // 변환을 캐싱하지 않아 다음 프레임에 변경된 위치가 반영되어야 한다.
    object.transform().pos = {56.f, 78.f};
    MENO_CHECK(object.sprite().drawParams().position == meno::Vec2f{56.f, 78.f});

    // offset은 부모 기준 위치이고 origin/flip은 그 위치를 바꾸지 않는다.
    object.transform().pos = {100.f, 100.f};
    object.transform().rotation = 0;
    object.transform().magnitude = 1;
    object.sprite().offset = {10.f, 10.f};
    MENO_CHECK(object.sprite().drawParams().position == meno::Vec2f{110.f, 110.f});
    MENO_CHECK(object.transform().pos == meno::Vec2f{100.f, 100.f});

    object.transform().rotation = 90;
    object.transform().magnitude = 2;
    const auto rotated = object.sprite().drawParams();
    MENO_CHECK(std::abs(rotated.position.x - 80.f) < 0.0001f);
    MENO_CHECK(std::abs(rotated.position.y - 120.f) < 0.0001f);

    detachedSprite.offset = {-10.f, 20.f};
    MENO_CHECK(detachedSprite.drawParams().position == meno::Vec2f{-10.f, 20.f});

    MENO_CHECK(object.getComponent<Health>() == nullptr);

    Health& health = object.addComponent<Health>(10);
    MENO_CHECK(health.parent == &object);
    MENO_CHECK(object.getComponent<Health>() == &health);
    MENO_CHECK(object.getComponent<Health>()->value == 10);
    MENO_CHECK(object.getComponent<Name>() == nullptr);

    Health& replacedHealth = object.addComponent<Health>(25);
    MENO_CHECK(replacedHealth.parent == &object);
    MENO_CHECK(object.getComponent<Health>() == &replacedHealth);
    MENO_CHECK(object.getComponent<Health>()->value == 25);

    Name& name = object.addComponent<Name>("player");
    MENO_CHECK(object.getComponent<Name>() == &name);
    MENO_CHECK(object.getComponent<Name>()->value == name.value);

    const meno::GameObject* constObject = scene.getGameObject(object.id());
    MENO_CHECK(constObject != nullptr);
    MENO_CHECK(&constObject->sprite() == &object.sprite());
    MENO_CHECK(&constObject->transform() == &object.transform());
    MENO_CHECK(constObject->getComponent<Health>() != nullptr);
    MENO_CHECK(constObject->getComponent<Health>()->value == 25);

    return meno::test::report();
}
