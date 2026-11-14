# 10장. 카메라와 좌표

이 장에서는 화면보다 넓은 월드를 카메라로 비춥니다. 플레이어를 따라가는 카메라, 확대와 축소, 마우스
좌표를 월드 좌표로 바꾸는 방법, 화면에 고정된 UI를 그리는 방법을 다룹니다.

## 완성 코드

2000×2000 크기의 월드를 방향키로 돌아다닙니다. 카메라가 플레이어를 따라가고, 마우스 휠로 확대하거나
축소하며, 클릭한 곳에는 표시가 남습니다. 왼쪽 위의 막대는 확대 배율을 보여 주는 UI입니다.

```cpp
#include <meno/Meno.hpp>

#include <algorithm>
#include <vector>

int main() {
    meno::Game game{{.application = {.framerateLimit = 60},
                     .windowSize = {800, 600},
                     .title = "Camera",
                     .clearColor = meno::Color::fromRgb(0x0D1117)}};
    const meno::Rectf world{{0.f, 0.f}, {2000.f, 2000.f}};
    meno::Vec2f player{1000.f, 1000.f};
    std::vector<meno::Vec2f> markers;
    meno::Camera2D camera{.center = player, .size = {800.f, 600.f}};
    game.renderer().setCamera(camera);

    meno::Scene& scene = game.scenes().createScene();
    scene.setUpdateHandler([&](double fixedDeltaTime) {
        const meno::Input& input = game.window().input();
        const float step = 300.f * static_cast<float>(fixedDeltaTime);
        if (input.isDown(meno::Key::Left)) {
            player.x -= step;
        }
        if (input.isDown(meno::Key::Right)) {
            player.x += step;
        }
        if (input.isDown(meno::Key::Up)) {
            player.y -= step;
        }
        if (input.isDown(meno::Key::Down)) {
            player.y += step;
        }
        player.x = std::clamp(player.x, world.left(), world.right());
        player.y = std::clamp(player.y, world.top(), world.bottom());

        // 휠로 확대·축소하고, 카메라가 플레이어를 따라가게 합니다.
        camera.zoom = std::clamp(camera.zoom + input.mouseWheelDelta() * 0.1f, 0.5f, 3.f);
        camera.center = player;
        game.renderer().setCamera(camera);

        // 클릭한 곳의 월드 좌표에 표시를 남깁니다.
        if (input.wasPressed(meno::MouseButton::Left)) {
            const meno::Vec2f mouse{input.mousePosition()};
            markers.push_back(game.renderer().screenToWorld(mouse));
        }
    });
    scene.setDrawHandler([&](meno::Renderer& renderer) {
        // 화면에 보이는 칸만 그립니다.
        const meno::Rectf visible = renderer.visibleWorldBounds();
        const float cell = 100.f;
        for (float y = world.top(); y < world.bottom(); y += cell) {
            for (float x = world.left(); x < world.right(); x += cell) {
                const meno::Rectf tile{{x, y}, {cell, cell}};
                if (!tile.intersects(visible)) {
                    continue;
                }
                const bool even =
                    (static_cast<int>(x / cell) + static_cast<int>(y / cell)) % 2 == 0;
                renderer.drawRect(tile, {.fill = even ? meno::Color::fromRgb(0x1E2430)
                                                      : meno::Color::fromRgb(0x253041)});
            }
        }
        for (const meno::Vec2f marker : markers) {
            renderer.drawCircle(marker, 6.f, {.fill = meno::colors::Yellow});
        }
        renderer.drawCircle(player, 16.f, {.fill = meno::Color::fromRgb(0x00B1ED)});

        // UI는 화면 좌표로 그립니다. 핸들러가 끝나면 Game이 월드 카메라를 되돌립니다.
        renderer.resetCamera();
        renderer.drawRect({{16.f, 16.f}, {40.f * camera.zoom, 8.f}}, {.fill = meno::colors::White});
    });

    (void)game.scenes().changeScene(scene.id());
    game.run();
}
```

## 월드 좌표와 화면 좌표

카메라를 정하지 않으면 그리기 좌표는 화면 좌표, 즉 창의 왼쪽 위를 원점으로 하는 픽셀 좌표입니다.
`renderer.setCamera(camera)`로 카메라를 정하면 그 뒤의 그리기 좌표는 **월드 좌표**로 해석됩니다.
월드 좌표의 단위와 방향은 화면 좌표와 같지만(y는 아래쪽으로 커짐), 화면에 비치는 영역은 카메라가
정합니다.

## Camera2D

`Camera2D`는 다음 필드를 가진 값 타입이며, 지정 초기화는 이 순서대로 씁니다.

| 필드 | 기본값 | 의미 |
|---|---|---|
| `center` | `{0, 0}` | 화면 가운데에 올 월드 좌표 |
| `size` | `{0, 0}` | 화면에 담을 월드 영역의 크기. `{0, 0}`이면 창의 픽셀 크기를 그대로 씀 |
| `zoom` | 1 | 배율. 1보다 크면 확대되어 보이는 영역이 `size / zoom`으로 좁아짐 |
| `rotation` | 0 | 회전 각도(도). 카메라가 시계 방향으로 돌면 화면 속 월드는 반시계 방향으로 돎 |
| `viewport` | `{0, 0, 1, 1}` | 창에서 이 카메라가 그릴 영역. 창 전체를 1로 보는 비율 |
| `fit` | `Letterbox` | `size`와 창의 종횡비가 다를 때의 처리 |

`size`는 "이 해상도로 설계했다"는 기준입니다. `size`를 정해 두면 창 크기가 바뀌어도 같은 영역이
비칩니다. 기본값 `Letterbox`는 종횡비를 지키며 남는 영역(띠)을 배경색으로 채우고, `Stretch`는 띠
없이 창을 채우는 대신 그림이 늘어납니다. 확대와 축소는 `size`를 바꾸지 말고 `zoom`으로 합니다.

## 카메라를 정하는 곳

`Renderer`는 카메라의 복사본을 저장합니다. 따라서 카메라 값을 바꿨다면 `setCamera()`를 다시 호출해야
반영됩니다.

- **월드 카메라**는 update 핸들러에서 정합니다. 위 예제처럼 플레이어를 움직인 뒤 `camera.center`를
  옮기고 `game.renderer().setCamera(camera)`를 호출합니다. 한 번 정한 카메라는 바꿀 때까지 계속
  유지됩니다.
- **UI**는 draw 핸들러에서 `renderer.resetCamera()`를 호출한 뒤 그립니다. `Game`은 draw 핸들러가
  끝나면 카메라를 핸들러가 호출되기 전의 상태로 되돌리므로, 다음 프레임의 월드 그리기에는 영향이
  없습니다.

GameObject의 Sprite는 draw 핸들러보다 먼저, 그 시점의 월드 카메라로 그려집니다.

## 마우스 위치를 월드 좌표로

`input.mousePosition()`은 창 기준 픽셀 좌표입니다. 카메라가 움직이거나 확대되어 있으면 이 좌표는
월드 좌표와 다르므로, `renderer.screenToWorld()`로 변환합니다. update 핸들러에서는
`game.renderer()`로 렌더러를 얻습니다.

```cpp
const meno::Vec2f mouse{game.window().input().mousePosition()};
const meno::Vec2f worldPoint = game.renderer().screenToWorld(mouse);
```

반대로 월드의 한 점이 창의 어디에 그려지는지는 `worldToScreen()`으로 구합니다. `Letterbox`의 띠
위에서 클릭하면 보이는 영역 밖의 월드 좌표가 나오므로, 필요하면
`meno::viewportPixels(camera, renderer.framebufferSize()).contains(mouse)`로 먼저 확인합니다.

## 보이는 것만 그리기

`renderer.visibleWorldBounds()`는 지금 카메라에 보이는 월드 영역을 돌려줍니다. 위 예제처럼 이 영역과
겹치지 않는 물체를 건너뛰면 넓은 월드도 가볍게 그릴 수 있습니다. 카메라가 회전해 있으면 실제보다
조금 넓은 영역을 돌려주므로, 보이는 물체를 놓치지는 않습니다.

## 화면 분할과 미니맵

`viewport`를 지정하면 카메라가 창의 일부에만 그립니다. 카메라마다 `setCamera()`를 호출한 뒤 월드를
다시 그리면 화면 분할이나 미니맵을 만들 수 있습니다.

```cpp
const meno::Vec2f hero{200.f, 300.f};
const meno::Vec2f rival{1800.f, 300.f};
const meno::Camera2D leftView{
    .center = hero, .size = {400.f, 600.f}, .viewport = {0.f, 0.f, 0.5f, 1.f}};
const meno::Camera2D rightView{
    .center = rival, .size = {400.f, 600.f}, .viewport = {0.5f, 0.f, 0.5f, 1.f}};

play.setDrawHandler([&](meno::Renderer& renderer) {
    for (const meno::Camera2D& view : {leftView, rightView}) {
        renderer.setCamera(view);
        // 월드를 카메라마다 다시 그립니다.
        renderer.drawCircle(hero, 16.f, {.fill = meno::colors::White});
        renderer.drawCircle(rival, 16.f, {.fill = meno::colors::Red});
    }
    renderer.resetCamera();
});
```

Scene이 자동으로 그리는 GameObject는 그 시점의 카메라 하나로만 그려집니다. 화면 분할에 GameObject를
함께 쓰려면 draw 핸들러에서 카메라마다 `object.draw(renderer)`를 호출해 직접 그립니다.

## 주의할 점

- 카메라 값을 바꾼 뒤에는 `setCamera()`를 다시 호출해야 화면에 반영됩니다.
- draw 핸들러 안에서 바꾼 카메라는 핸들러가 끝나면 되돌아갑니다. 월드 카메라는 update 핸들러에서
  정합니다.
- `Camera2D`의 `rotation`은 스프라이트의 `rotation`과 반대로 보입니다. 같은 +45도라도 카메라를
  돌리면 화면 속 월드는 반시계 방향으로 돕니다.
- 지정 초기화는 `center`, `size`, `zoom`, `rotation`, `viewport`, `fit` 순서로 씁니다.

## 관련 예제와 API

- 예제:
  [render_basics](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/render_basics/main.cpp)
  (월드 카메라와 미니맵을 함께 보여 줍니다)
- API: [Camera2D](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_camera2_d.html),
  [Renderer](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_renderer.html),
  [수학 모듈](https://seoultech-fiat-lux.github.io/Meno/group__math.html)(`CameraFit`,
  `viewportPixels`, `screenToWorld` 등)

이전 장: [9장. 리소스 관리](09-resources.md) · 다음 장: [11장. 스프라이트 애니메이션](11-animation.md)
