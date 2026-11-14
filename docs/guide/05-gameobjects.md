# 5장. GameObject와 컴포넌트

1부에서는 게임 상태를 변수에 두고 draw 핸들러에서 직접 그렸습니다. 이 장에서는 화면에 놓이는 물체를
`GameObject`로 만들고, 위치와 그림, 직접 정의한 데이터를 컴포넌트로 붙여 관리합니다.

## 완성 코드

눈송이가 계속 내리는 장면입니다. 눈송이마다 GameObject를 하나씩 만들고, 화면 아래로 벗어나면
지웁니다.

```cpp
#include <meno/Meno.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

// 초당 이동 거리를 담는 컴포넌트입니다.
struct Velocity : meno::Component {
    meno::Vec2f value;
    explicit Velocity(meno::Vec2f initial) : value(initial) {}
};

int main() {
    meno::Game game{{.application = {.framerateLimit = 60},
                     .windowSize = {640, 480},
                     .title = "Snow",
                     .clearColor = meno::Color::fromRgb(0x0D1117)}};
    const meno::Vec2f field{game.window().size()};

    // 모든 눈송이가 함께 쓰는 4×4 흰색 텍스처입니다.
    std::array<std::uint8_t, 4 * 4 * 4> pixels{};
    pixels.fill(255);
    std::shared_ptr<const meno::Texture> flake;
    if (auto texture = meno::Texture::fromPixels({4, 4}, pixels.data())) {
        flake = std::make_shared<const meno::Texture>(std::move(*texture));
    }

    std::mt19937 random{std::random_device{}()};
    std::uniform_real_distribution<float> randomX{0.f, field.x};
    std::uniform_real_distribution<float> randomScale{1.f, 3.f};
    std::vector<meno::GameObject::GameObjectID> flakes;
    double untilNext = 0.0;

    meno::Scene& scene = game.scenes().createScene();
    scene.setUpdateHandler([&](double fixedDeltaTime) {
        // 0.1초마다 눈송이를 하나씩 만듭니다.
        untilNext -= fixedDeltaTime;
        if (untilNext <= 0.0 && flake) {
            untilNext = 0.1;
            const float scale = randomScale(random);
            meno::GameObject& object = scene.createGameObject();
            object.transform().position = {randomX(random), -16.f};
            object.transform().scale = scale;
            object.sprite().setTexture(flake);
            object.addComponent<Velocity>(meno::Vec2f{0.f, 40.f * scale});
            (void)scene.setLayer(object.id(), scale > 2.f ? 1 : 0); // 큰 눈송이를 앞에 그립니다.
            flakes.push_back(object.id());
        }

        // 눈송이를 움직이고, 화면 아래로 벗어난 것은 지웁니다.
        const float dt = static_cast<float>(fixedDeltaTime);
        std::erase_if(flakes, [&](meno::GameObject::GameObjectID id) {
            meno::GameObject* object = scene.getGameObject(id);
            if (object == nullptr) {
                return true;
            }
            if (const Velocity* velocity = object->getComponent<Velocity>()) {
                object->transform().position += velocity->value * dt;
            }
            if (object->transform().position.y > field.y) {
                (void)scene.deleteGameObject(id);
                return true;
            }
            return false;
        });
    });
    scene.setDrawHandler([&](meno::Renderer& renderer) {
        // draw 핸들러에서 그린 것은 GameObject보다 위에 놓이므로, 눈송이가 땅에 가려집니다.
        renderer.drawRect({{0.f, field.y - 40.f}, {field.x, 40.f}},
                          {.fill = meno::Color::fromRgb(0x334155)});
    });

    (void)game.scenes().changeScene(scene.id());
    game.run();
}
```

## GameObject의 기본 구성

`scene.createGameObject()`는 GameObject를 만들어 Scene에 넣고, 그 참조를 돌려줍니다. GameObject는
Scene이 소유하며, 모든 GameObject는 두 가지 기본 컴포넌트를 가집니다.

| 기본 컴포넌트 | 접근 | 내용 |
|---|---|---|
| `Transform` | `object.transform()` | 위치(`position`), 배율(`scale`), 회전(`rotation`) |
| `Sprite` | `object.sprite()` | 그림(텍스처)과 그리기 옵션(`offset`, `origin`, `tint` 등) |

`transform().position`은 GameObject의 왼쪽 위 꼭짓점(로컬 원점)이 놓일 위치입니다. 배율과 회전도 이
점을 중심으로 적용됩니다. `scale`은 가로와 세로에 함께 쓰이는 값 하나입니다.

Sprite는 텍스처가 있을 때만 그려집니다. `sprite().setTexture()`에는
`ResourceManager::loadTexture()`가 돌려준 `shared_ptr`을 넘깁니다. 위 예제처럼 코드로 만든 텍스처를
여러 GameObject가 함께 쓰게 하려면 `std::make_shared`로 감싸서 넘깁니다.

## 그리는 순서와 레이어

Scene은 GameObject를 다음 순서로 그린 뒤에 draw 핸들러를 호출합니다.

1. 레이어가 작은 것부터 그립니다. 레이어의 기본값은 0이며 `scene.setLayer(id, layer)`로 바꿉니다.
2. 레이어가 같으면 먼저 만든 것(ID가 작은 것)부터 그립니다.

나중에 그린 것이 위에 놓이므로, 레이어가 큰 GameObject가 앞에 보입니다. draw 핸들러는 GameObject를
모두 그린 뒤에 호출되므로, 점수와 같은 UI는 여기서 그립니다.

## 컴포넌트 붙이기

위치와 그림 외의 데이터는 `meno::Component`를 상속한 구조체로 정의하여 붙입니다.

```cpp
struct Health : meno::Component {
    int value = 3;
};

meno::GameObject& player = play.createGameObject();
player.addComponent<Health>();

if (Health* health = player.getComponent<Health>()) {
    health->value -= 1;
}
```

- `addComponent<T>(인자...)`는 T를 만들어 붙이고 그 참조를 돌려줍니다. 인자는 T의 생성자에 그대로
  전달됩니다.
- `getComponent<T>()`는 붙어 있는 T를 찾아 포인터로 돌려주며, 없으면 `nullptr`입니다.
- 컴포넌트는 타입마다 하나씩만 붙습니다. 같은 타입을 다시 붙이면 이전 컴포넌트는 파괴됩니다.
- 붙인 컴포넌트는 GameObject가 소유하며, 떼어 내는 함수는 없습니다.

meno의 컴포넌트에는 매 프레임마다 호출되는 함수가 없습니다. 컴포넌트는 데이터만 담고, 그 데이터를
쓰는 동작은 Scene의 update 핸들러에서 처리합니다. 위 예제에서도 update 핸들러가 `Velocity`를 꺼내
위치에 더합니다.

## GameObject 목록 관리하기

Scene은 GameObject 목록을 돌려주는 함수를 제공하지 않습니다. 여러 GameObject를 함께 다루려면 위
예제의 `flakes`처럼 ID를 직접 모아 두고, `scene.getGameObject(id)`로 꺼내 씁니다.
`getGameObject()`는 그 ID의 GameObject가 없으면 `nullptr`을 돌려주므로, 이미 지운 객체를 실수로 쓰는
일을 막을 수 있습니다.

GameObject는 `scene.deleteGameObject(id)`로 지웁니다. 지우면 붙어 있던 컴포넌트도 함께 파괴됩니다.

## update 중에 만들고 지우기

update 핸들러 안에서 GameObject를 만들거나 지우면, 변경 사항은 그 update가 끝날 때 반영됩니다.

| 동작 | update 핸들러 안 | 핸들러 밖 |
|---|---|---|
| `createGameObject()` | 바로 쓸 수 있지만, 그 update가 끝난 뒤부터 그려짐 | 바로 그려짐 |
| `deleteGameObject(id)` | 그 update가 끝날 때 파괴됨 | 바로 파괴됨 |

update 안에서 지운 GameObject는 그 update가 끝날 때까지 남아 있으므로, 같은 update 안에서는 계속
`getGameObject()`로 찾을 수 있습니다. 반대로 핸들러 밖에서 지우면 바로 파괴되므로, 그 객체를
가리키던 참조와 포인터도 그 즉시 쓸 수 없게 됩니다.

## 주의할 점

- `getComponent<T>()`는 정확히 같은 타입만 찾습니다. 예를 들어 `BoxCollider`를 붙였다면
  `getComponent<meno::Collider>()`는 `nullptr`을 돌려줍니다.
- 같은 타입의 컴포넌트를 다시 붙이면 이전 컴포넌트는 파괴되므로, 이전에 받아 둔 참조를 쓰면 안
  됩니다.
- `Transform`과 `Sprite`는 `addComponent()`로 붙일 수 없습니다. `transform()`과 `sprite()`로 씁니다.
- GameObject의 참조나 포인터를 오래 들고 있어야 한다면 ID를 대신 저장하고, 쓸 때마다
  `getGameObject()`로 꺼냅니다.

## 관련 예제와 API

- 예제:
  [gameobject_demo](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/gameobject_demo/main.cpp)
  (Scene, GameObject, Sprite로 장면을 구성합니다)
- API: [GameObject](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_game_object.html),
  [Component](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_component.html),
  [Transform](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_transform.html),
  [Sprite](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_sprite.html),
  [Scene](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_scene.html)

이전 장: [4장. 입력](04-input.md) · 다음 장: [6장. 충돌 판정](06-collision.md)
