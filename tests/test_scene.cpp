#include <meno/core/Scene.hpp>

#include "check.hpp"

#include <type_traits>
#include <utility>

int main() {
    static_assert(!std::is_copy_constructible_v<meno::Scene>);
    static_assert(!std::is_copy_assignable_v<meno::Scene>);
    static_assert(std::is_move_constructible_v<meno::Scene>);
    static_assert(std::is_move_assignable_v<meno::Scene>);

    meno::Scene scene;

    meno::GameObject& first = scene.createGameObject();
    const auto firstId = first.id();

    MENO_CHECK(firstId == 1);
    MENO_CHECK(scene.getGameObject(firstId) == &first);
    MENO_CHECK(scene.getGameObject(999) == nullptr);

    meno::GameObject& second = scene.createGameObject();
    const auto secondId = second.id();

    MENO_CHECK(secondId != firstId);
    MENO_CHECK(secondId == 2);
    MENO_CHECK(scene.getGameObject(firstId) == &first);
    MENO_CHECK(scene.getGameObject(secondId) == &second);

    const meno::Scene& constScene = scene;
    MENO_CHECK(constScene.getGameObject(firstId) == &first);
    MENO_CHECK(constScene.getGameObject(999) == nullptr);

    MENO_CHECK(scene.deleteGameObject(firstId));
    MENO_CHECK(scene.getGameObject(firstId) == nullptr);
    MENO_CHECK(scene.getGameObject(secondId) == &second);
    MENO_CHECK(!scene.deleteGameObject(firstId));

    meno::GameObject& third = scene.createGameObject();
    const auto thirdId = third.id();

    MENO_CHECK(thirdId != firstId);
    MENO_CHECK(thirdId != secondId);
    MENO_CHECK(thirdId == 3);
    MENO_CHECK(scene.getGameObject(thirdId) == &third);

    meno::Scene movedScene = std::move(scene);
    MENO_CHECK(movedScene.getGameObject(secondId) != nullptr);
    MENO_CHECK(movedScene.getGameObject(thirdId) != nullptr);
    MENO_CHECK(movedScene.getGameObject(firstId) == nullptr);

    return meno::test::report();
}
