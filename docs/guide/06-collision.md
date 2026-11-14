# 6장. 충돌 판정

이 장에서는 GameObject에 콜라이더를 붙여 서로 겹치는지 판정하고, 겹쳤을 때 막거나 줍는 처리를
구현합니다.

## 완성 코드

방향키로 파란 상자를 움직여 노란 동전을 모읍니다. 가운데 벽은 통과할 수 없습니다.

```cpp
#include <meno/Meno.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// size 크기의 단색 텍스처를 만듭니다. round가 true면 원 바깥을 투명하게 둡니다.
std::shared_ptr<const meno::Texture> makeTexture(meno::Vec2u size, meno::Color color, bool round) {
    std::vector<std::uint8_t> pixels(std::size_t{size.x} * size.y * 4);
    const float radius = static_cast<float>(size.x) / 2.f;
    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            const float dx = static_cast<float>(x) + 0.5f - radius;
            const float dy = static_cast<float>(y) + 0.5f - radius;
            const bool inside = !round || dx * dx + dy * dy <= radius * radius;
            const std::size_t i = (std::size_t{y} * size.x + x) * 4;
            pixels[i] = color.r;
            pixels[i + 1] = color.g;
            pixels[i + 2] = color.b;
            pixels[i + 3] = inside ? color.a : std::uint8_t{0};
        }
    }
    auto texture = meno::Texture::fromPixels(size, pixels.data());
    return texture ? std::make_shared<const meno::Texture>(std::move(*texture)) : nullptr;
}

int main() {
    meno::Game game{{.application = {.framerateLimit = 60},
                     .windowSize = {640, 480},
                     .title = "Coins",
                     .clearColor = meno::Color::fromRgb(0x1E2430)}};
    const auto font = game.resources().loadFont("assets/fonts/game.ttf");
    meno::Scene& scene = game.scenes().createScene();

    meno::GameObject& player = scene.createGameObject();
    player.transform().position = {80.f, 220.f};
    player.sprite().setTexture(makeTexture({32, 32}, meno::Color::fromRgb(0x00B1ED), false));
    player.addComponent<meno::BoxCollider>(meno::Vec2f{0.f, 0.f}, meno::Vec2f{32.f, 32.f});

    meno::GameObject& wall = scene.createGameObject();
    wall.transform().position = {304.f, 120.f};
    wall.sprite().setTexture(makeTexture({32, 240}, meno::colors::Gray, false));
    wall.addComponent<meno::BoxCollider>(meno::Vec2f{0.f, 0.f}, meno::Vec2f{32.f, 240.f});

    const auto coinTexture = makeTexture({16, 16}, meno::colors::Yellow, true);
    std::vector<meno::GameObject::GameObjectID> coins;
    for (const meno::Vec2f position :
         {meno::Vec2f{200.f, 100.f}, meno::Vec2f{200.f, 360.f}, meno::Vec2f{440.f, 100.f},
          meno::Vec2f{440.f, 360.f}, meno::Vec2f{540.f, 230.f}}) {
        meno::GameObject& coin = scene.createGameObject();
        coin.transform().position = position;
        coin.sprite().setTexture(coinTexture);
        coin.addComponent<meno::CircleCollider>(meno::Vec2f{8.f, 8.f}, 8.f); // 중심과 반지름
        coins.push_back(coin.id());
    }
    int collected = 0;

    scene.setUpdateHandler([&](double fixedDeltaTime) {
        const meno::Input& input = game.window().input();
        const float step = 240.f * static_cast<float>(fixedDeltaTime);
        meno::Vec2f move{0.f, 0.f};
        if (input.isDown(meno::Key::Left)) {
            move.x -= step;
        }
        if (input.isDown(meno::Key::Right)) {
            move.x += step;
        }
        if (input.isDown(meno::Key::Up)) {
            move.y -= step;
        }
        if (input.isDown(meno::Key::Down)) {
            move.y += step;
        }

        // 축마다 따로 움직여 보고, 벽과 겹치면 그 축의 이동만 되돌립니다.
        meno::Vec2f& position = player.transform().position;
        position.x += move.x;
        if (meno::intersects(player, wall)) {
            position.x -= move.x;
        }
        position.y += move.y;
        if (meno::intersects(player, wall)) {
            position.y -= move.y;
        }

        // 닿은 동전을 지우고 점수를 올립니다.
        std::erase_if(coins, [&](meno::GameObject::GameObjectID id) {
            const meno::GameObject* coin = scene.getGameObject(id);
            if (coin == nullptr || !meno::intersects(player, *coin)) {
                return false;
            }
            (void)scene.deleteGameObject(id);
            ++collected;
            return true;
        });
    });
    scene.setDrawHandler([&](meno::Renderer& renderer) {
        if (font) {
            const std::string text = coins.empty() ? "Clear!" : std::to_string(collected) + " / 5";
            renderer.drawText(*font, text, {16.f, 16.f});
        }
    });

    (void)game.scenes().changeScene(scene.id());
    game.run();
}
```

## 콜라이더 붙이기

콜라이더는 GameObject에 붙이는 컴포넌트입니다. 사각형 콜라이더 `BoxCollider`와 원형 콜라이더
`CircleCollider` 두 가지가 있습니다.

| 콜라이더 | 생성자 | `offset`이 가리키는 곳 |
|---|---|---|
| `BoxCollider` | `BoxCollider(offset, size)` | 사각형의 왼쪽 위 꼭짓점 |
| `CircleCollider` | `CircleCollider(offset, radius)` | 원의 중심 |

`offset`과 크기는 GameObject의 로컬 좌표로 지정합니다. 로컬 좌표의 원점은 GameObject의 왼쪽 위
꼭짓점이므로, 32×32 텍스처를 쓰는 GameObject에 같은 크기의 BoxCollider를 붙이려면 `offset`을
`{0, 0}`, `size`를 `{32, 32}`로 지정합니다. 마찬가지로 16×16 동전의 중심은 `{8, 8}`입니다.

콜라이더는 GameObject의 Transform을 따릅니다. `transform().scale`을 키우면 콜라이더도 함께 커지고,
`rotation`을 주면 BoxCollider도 회전합니다. 따라서 텍스처를 4배로 키운 GameObject라도 콜라이더의
크기는 텍스처의 원래 픽셀 크기로 지정합니다.

## 겹치는지 판정하기

`meno::intersects(a, b)`는 두 GameObject의 콜라이더가 하나라도 겹치면 true를 돌려줍니다. 경계만
맞닿은 경우는 겹치지 않은 것으로 봅니다. 점 하나가 사각형 안에 있는지는
`BoxCollider::contains(worldPoint)`로 확인합니다.

```cpp
if (const auto* box = player.getComponent<meno::BoxCollider>()) {
    const meno::Vec2f mouse{game.window().input().mousePosition()};
    if (box->contains(mouse)) {
        // 마우스가 player 위에 있습니다.
    }
}
```

GameObject 없이 사각형 두 개만 비교할 때는 4장의 퐁처럼 `Rectf::intersects()`를 씁니다.

## 겹쳤을 때 처리하기

meno는 겹침 여부만 판정합니다. 겹친 물체를 밀어내거나 속도를 바꾸는 처리는 게임 코드가 맡습니다.
자주 쓰는 방법은 다음과 같습니다.

- **막기:** 위 예제처럼 움직인 뒤 겹치면 이동을 되돌립니다. x축과 y축을 따로 검사하면 벽에 닿은
  채로도 벽을 따라 미끄러지듯 움직일 수 있습니다.
- **착지:** 떨어지던 물체가 바닥과 겹치면 위치를 바닥 위로 맞추고 세로 속도를 0으로 둡니다. 이번
  update에서 움직이기 전에 바닥보다 위에 있었는지도 함께 확인하면, 옆에서 부딪힌 것을 착지로
  오인하지 않습니다.
  [jumping_box](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/jumping_box/main.cpp)
  예제가 이 방식을 씁니다.
- **튕기기:** 4장의 퐁처럼 부딪힌 축의 속도 부호를 바꿉니다. 부딪힌 물체에서 멀어지는 방향으로
  부호를 정하면, 한 번 겹친 상태가 여러 update 동안 이어져도 다시 안쪽으로 튕기지 않습니다.
- **줍기:** 위 예제의 동전처럼 겹친 GameObject를 지우고 점수를 올립니다.

## 주의할 점

- `intersects()`는 `BoxCollider`와 `CircleCollider`만 검사합니다. `Collider`를 상속해 직접 만든
  타입은 판정에 쓰이지 않습니다.
- 크기나 반지름이 0 이하인 콜라이더와 배율이 0인 GameObject에 붙은 콜라이더는 어떤 것과도 겹치지
  않습니다.
- `getComponent<meno::Collider>()`로는 BoxCollider나 CircleCollider를 찾을 수 없습니다.
  `getComponent<meno::BoxCollider>()`처럼 정확한 타입으로 찾습니다.
- 물체가 아주 빠르면 update 한 번에 얇은 벽을 통째로 건너뛸 수 있습니다. 이를 막으려면 속도를
  줄이거나 벽을 두껍게 만듭니다.

## 관련 예제와 API

- 예제:
  [jumping_box](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/jumping_box/main.cpp)
  (중력과 착지를 게임 코드로 처리합니다)
- API: [Collider](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_collider.html),
  [BoxCollider](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_box_collider.html),
  [CircleCollider](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_circle_collider.html),
  [씬 모듈](https://seoultech-fiat-lux.github.io/Meno/group__scene.html)(`intersects`),
  [Rect](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_rect.html)

이전 장: [5장. GameObject와 컴포넌트](05-gameobjects.md) · 다음 장: [7장. 씬 전환과 씬 상태](07-scenes.md)
