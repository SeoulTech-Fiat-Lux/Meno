#include <meno/core/GameObject.hpp>
#include <meno/core/Scene.hpp>

#include "check.hpp"

#include <type_traits>

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
    static_assert(!std::is_copy_constructible_v<meno::GameObject>);
    static_assert(!std::is_copy_assignable_v<meno::GameObject>);

    static_assert(CanAddComponent<Health>);
    static_assert(!CanAddComponent<meno::Transform>);
    static_assert(!CanAddComponent<meno::Sprite>);

    meno::Scene scene;
    meno::GameObject& object = scene.createGameObject();

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

    MENO_CHECK(object.getComponent<Health>() == nullptr);

    Health& health = object.addComponent<Health>(10);
    MENO_CHECK(object.getComponent<Health>() == &health);
    MENO_CHECK(object.getComponent<Health>()->value == 10);
    MENO_CHECK(object.getComponent<Name>() == nullptr);

    Health& replacedHealth = object.addComponent<Health>(25);
    MENO_CHECK(object.getComponent<Health>() == &replacedHealth);
    MENO_CHECK(object.getComponent<Health>()->value == 25);

    Name& name = object.addComponent<Name>("player");
    MENO_CHECK(object.getComponent<Name>() == &name);
    MENO_CHECK(object.getComponent<Name>()->value == name.value);

    const meno::GameObject* constObject = scene.getGameObject(object.id());
    MENO_CHECK(constObject != nullptr);
    MENO_CHECK(constObject->getComponent<Health>() != nullptr);
    MENO_CHECK(constObject->getComponent<Health>()->value == 25);

    return meno::test::report();
}
