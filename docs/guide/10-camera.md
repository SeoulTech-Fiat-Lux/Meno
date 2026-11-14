# 10장. 카메라와 좌표

이 장에서는 화면보다 넓은 월드를 카메라로 비춥니다. 플레이어를 따라가는 카메라, 확대와 축소, 마우스
좌표를 월드 좌표로 바꾸는 방법, 화면에 고정된 UI를 그리는 방법을 다룹니다.

## 완성 코드

2000×2000 크기의 월드를 방향키로 돌아다닙니다. 카메라가 플레이어를 따라가고, 마우스 휠로 확대하거나
축소할 수 있으며, 클릭한 곳에는 표시가 남습니다. 왼쪽 위의 막대는 확대 배율을 보여 주는 UI입니다.

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
    // UI를 그리는 카메라입니다. 월드 카메라와 size가 같고 움직이지 않습니다.
    const meno::Camera2D hud{.center = {400.f, 300.f}, .size = {800.f, 600.f}};

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

        // UI는 HUD 카메라로 그립니다. 핸들러가 끝나면 Game이 월드 카메라를 되돌립니다.
        renderer.setCamera(hud);
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
| `viewport` | `{0, 0, 1, 1}` | 창에서 이 카메라가 차지할 영역. 창 전체를 1로 보는 비율 |
| `fit` | `Letterbox` | `size`와 창의 종횡비가 다를 때의 처리 |

`size`는 "이 해상도로 설계했다"는 기준입니다. `size`를 정해 두면 창 크기가 바뀌어도 같은 영역이
비칩니다. `fit`의 기본값인 `Letterbox`는 종횡비를 지키는 대신 창의 남는 부분을 띠로 비워 두고,
`Stretch`는 띠 없이 창을 채우는 대신 그림을 늘립니다. 확대와 축소는 `size`를 바꾸는 대신 `zoom`으로
합니다.

띠는 프레임마다 화면을 지우는 색인 `GameConfig::clearColor`로 채워집니다. 띠와 배경의 색을 다르게
하려면 `clearColor`는 띠 색으로 두고, 배경은 게임 화면이나 월드 크기의 사각형으로 그립니다. draw
핸들러에서 그린 것은 GameObject 위에 놓이므로, GameObject를 함께 쓴다면 배경은 레이어가 가장 작은
GameObject의 Sprite로 둡니다.

## 카메라를 정하는 곳

`Renderer`는 카메라의 복사본을 저장합니다. 따라서 카메라 값을 바꿨다면 `setCamera()`를 다시 호출해야
반영됩니다.

- **월드 카메라**는 update 핸들러에서 정합니다. 위 예제처럼 플레이어를 움직인 뒤 `camera.center`를
  옮기고 `game.renderer().setCamera(camera)`를 호출합니다. 한 번 정한 카메라는 다시 바꿀 때까지
  유지됩니다.
- **UI**는 draw 핸들러에서 HUD 카메라(다음 절)로 바꾼 뒤에 그립니다. `Game`은 draw 핸들러가 끝나면
  카메라를 핸들러가 호출되기 전의 상태로 되돌리므로, 다음 프레임의 월드 그리기에는 영향이 없습니다.

GameObject의 Sprite는 draw 핸들러보다 먼저, 그 시점의 월드 카메라로 그려집니다.

## 화면에 고정된 UI

점수나 체력 막대처럼 카메라가 움직여도 화면의 같은 자리에 있어야 하는 UI는 **HUD 카메라**로
그립니다. HUD 카메라는 월드 카메라와 `size`가 같고, `center`를 `size`의 절반에 고정한 카메라입니다.
이 카메라로 그리면 UI 좌표의 원점은 띠를 뺀 게임 화면의 왼쪽 위가 됩니다. 창 크기가 바뀌면 UI도 게임
화면과 함께 커지거나 작아지므로, 창을 크게 키우면 글자가 조금 흐려질 수 있습니다.

UI의 클릭을 판정할 때는 마우스 위치도 HUD 카메라의 좌표로 바꿉니다. update 핸들러에서
`game.renderer().screenToWorld()`는 월드 카메라를 기준으로 변환하므로, 카메라를 인자로 받는
`meno::screenToWorld()`를 씁니다.

```cpp
const meno::Camera2D hud{.center = {400.f, 300.f}, .size = {800.f, 600.f}};
const meno::Rectf button{{16.f, 64.f}, {160.f, 48.f}};

play.setUpdateHandler([&](double) {
    // 마우스 위치를 HUD 카메라의 좌표로 바꿔서 판정합니다.
    const meno::Input& input = game.window().input();
    const meno::Vec2f mouse{input.mousePosition()};
    const meno::Vec2f point = meno::screenToWorld(hud, game.renderer().framebufferSize(), mouse);
    if (input.wasPressed(meno::MouseButton::Left) && button.contains(point)) {
        // 버튼을 눌렀습니다.
    }
});
play.setDrawHandler([&](meno::Renderer& renderer) {
    // 월드를 그린 뒤에 HUD 카메라로 바꿉니다.
    renderer.setCamera(hud);
    renderer.drawText(*font, "SCORE 0", {16.f, 16.f}, {.characterSize = 32});
    renderer.drawRect(button, {.fill = meno::colors::Gray});
});
```

`renderer.resetCamera()`는 그리기 좌표를 창의 픽셀 좌표로 되돌립니다. 이 좌표는 띠를 따르지
않으므로, 창의 종횡비가 바뀌면 (16, 16)에 그린 UI가 게임 화면 밖의 띠 위에 놓일 수 있습니다.
`resetCamera()`는 창 모서리에 붙어야 하는 표시를 그리거나, `viewportPixels()`로 구한 픽셀 영역에
그릴 때 씁니다.

## 마우스 위치를 월드 좌표로

`input.mousePosition()`은 창 기준 픽셀 좌표를 돌려줍니다. 카메라가 움직였거나 확대되어 있으면 이
좌표는 월드 좌표와 다르므로, `renderer.screenToWorld()`로 변환합니다. update 핸들러에서는
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

`viewport`를 지정하면 카메라가 비추는 장면이 창의 일부에만 그려집니다. 카메라마다 `setCamera()`를
호출한 뒤 월드를 다시 그리면 화면 분할이나 미니맵을 만들 수 있습니다.

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

Scene이 자동으로 그리는 GameObject에는 그 시점의 카메라 하나만 적용됩니다. 화면 분할에 GameObject를
함께 쓰려면 draw 핸들러에서 카메라마다 `object.draw(renderer)`를 호출해 직접 그립니다.

## 레터박스와 화면 분할

`viewport`는 창 전체에 대한 비율이고, `Letterbox`는 카메라마다 따로 적용됩니다. 따라서 위 예제의 두
화면은 각자 띠를 가지며, 창을 가로로 늘리면 두 화면 사이에도 띠가 생깁니다. 두 화면을 하나의 게임
화면 안에 붙여 두려면, 게임 화면이 창에서 차지하는 픽셀 영역을 `viewportPixels()`로 구한 뒤 그
안에서 `viewport`를 계산합니다.

```cpp
// 800×600 게임 화면 안의 사각형(게임 화면 좌표)을 창 전체에 대한 viewport 비율로 바꿉니다.
const auto viewportInGame = [](meno::Vec2u framebuffer, const meno::Rectf& inGame) {
    const meno::Rectf area =
        meno::viewportPixels(meno::Camera2D{.size = {800.f, 600.f}}, framebuffer);
    const float scaleX = area.size.x / 800.f;
    const float scaleY = area.size.y / 600.f;
    const auto width = static_cast<float>(framebuffer.x);
    const auto height = static_cast<float>(framebuffer.y);
    return meno::Rectf{(area.left() + inGame.left() * scaleX) / width,
                       (area.top() + inGame.top() * scaleY) / height,
                       inGame.size.x * scaleX / width, inGame.size.y * scaleY / height};
};
const meno::Camera2D hud{.center = {400.f, 300.f}, .size = {800.f, 600.f}};

play.setDrawHandler([&](meno::Renderer& renderer) {
    // 창 크기가 바뀔 수 있으므로 viewport는 프레임마다 계산합니다.
    const meno::Vec2u framebuffer = renderer.framebufferSize();
    meno::Camera2D leftView{.center = {200.f, 300.f}, .size = {400.f, 600.f}};
    meno::Camera2D rightView{.center = {1800.f, 300.f}, .size = {400.f, 600.f}};
    leftView.viewport = viewportInGame(framebuffer, {0.f, 0.f, 400.f, 600.f});
    rightView.viewport = viewportInGame(framebuffer, {400.f, 0.f, 400.f, 600.f});
    for (const meno::Camera2D& view : {leftView, rightView}) {
        renderer.setCamera(view);
        // 월드를 카메라마다 다시 그립니다.
    }
    // 두 화면의 경계선처럼 게임 화면에 붙는 UI는 HUD 카메라로 그립니다.
    renderer.setCamera(hud);
    renderer.drawRect({{398.f, 0.f}, {4.f, 600.f}});
});
```

미니맵처럼 게임 화면의 일부에만 월드를 비추는 카메라도 같은 방법으로 `viewport`를 정합니다.

## 주의할 점

- 카메라 값을 바꾼 뒤에는 `setCamera()`를 다시 호출해야 화면에 반영됩니다.
- draw 핸들러 안에서 바꾼 카메라는 핸들러가 끝나면 되돌아갑니다. 따라서 월드 카메라는 update
  핸들러에서 정합니다.
- `resetCamera()`의 화면 좌표는 띠를 따르지 않습니다. 게임 화면에 붙어 있어야 하는 UI는 HUD 카메라로
  그립니다.
- 띠는 `clearColor`로 채워집니다. 배경색이 띠와 다르면 배경을 사각형으로 그립니다.
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
