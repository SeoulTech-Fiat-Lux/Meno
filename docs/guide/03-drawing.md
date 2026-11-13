# 3장. 그리기

이 장에서는 draw 핸들러에서 도형과 텍스트, 이미지를 그리고 색을 지정하는 방법을 다룹니다. 그리기는
모두 핸들러가 인자로 받는 `meno::Renderer`로 합니다.

## 완성 코드

에셋 파일 없이 실행되는 그리기 연습 예제입니다. 폰트 파일이 없으면 글자만 그려지지 않습니다.

```cpp
#include <meno/Meno.hpp>

#include <array>
#include <cstdint>
#include <optional>

int main() {
    meno::Game game{{.application = {.framerateLimit = 60},
                     .windowSize = {640, 480},
                     .title = "Drawing",
                     .clearColor = meno::Color::fromRgb(0x1E2430)}};

    // 파일 대신 픽셀 배열로 만든 2×2 체크무늬 텍스처입니다.
    const std::array<std::uint8_t, 16> pixels{
        255, 255, 255, 255, 0,   177, 237, 255, //
        0,   177, 237, 255, 255, 255, 255, 255,
    };
    const std::optional<meno::Texture> checker = meno::Texture::fromPixels({2, 2}, pixels.data());
    const auto font = game.resources().loadFont("assets/fonts/game.ttf");

    meno::Scene& scene = game.scenes().createScene();
    scene.setDrawHandler([&](meno::Renderer& renderer) {
        renderer.drawRect({{40.f, 40.f}, {160.f, 100.f}}, {.fill = meno::colors::White});
        renderer.drawCircle({320.f, 90.f}, 50.f,
                            {.fill = meno::colors::Transparent,
                             .outline = meno::Color::fromRgb(0x00B1ED),
                             .outlineThickness = 4.f});
        renderer.drawLine({440.f, 40.f}, {600.f, 140.f}, {.thickness = 3.f});
        renderer.drawPolygon({120.f, 260.f}, {{0.f, -50.f}, {50.f, 40.f}, {-50.f, 40.f}},
                             {.fill = meno::colors::Yellow});
        renderer.drawSector({320.f, 260.f}, 50.f, -90.f, 270.f, {.fill = meno::colors::Gray});
        if (checker) {
            renderer.draw(*checker, {.position = {460.f, 210.f}, .scale = {50.f, 50.f}});
        }
        if (font) {
            renderer.drawText(*font, "Hello, meno", {40.f, 380.f}, {.characterSize = 32});
        }
    });

    (void)game.scenes().changeScene(scene.id());
    game.run();
}
```

## 좌표와 그리는 순서

- 단위는 픽셀이고 원점은 창의 왼쪽 위입니다. x는 오른쪽으로, **y는 아래쪽으로** 갈수록 커집니다.
- 각도는 도(degree) 단위이며 **시계 방향**이 양수입니다. 0도는 오른쪽 방향입니다.
- 그리기 함수를 부른 순서대로 그리므로 나중에 그린 것이 위에 놓입니다. 따라서 배경을 먼저, 캐릭터와
  UI를 나중에 그립니다.

카메라를 쓰면 그리기 좌표를 "월드 좌표"로 해석해, 화면에 보이는 영역을 옮기거나 확대할 수 있습니다.
카메라를 정하지 않으면 위의 화면 좌표를 그대로 씁니다.

## 도형

| 함수 | 그리는 것 |
|---|---|
| `drawRect(rect, params)` | 사각형. 기본값에서는 `rect.position`이 왼쪽 위 꼭짓점 |
| `drawCircle(center, radius, params)` | 원 |
| `drawLine(from, to, params)` | 두께가 있는 선. 옵션은 `LineParams{.color, .thickness}` |
| `drawPolygon(position, points, params)` | 점을 차례로 이은 다각형. 각 점은 `position` 기준 좌표 |
| `drawSector(center, radius, startDeg, sweepDeg, params)` | 부채꼴(쿨다운 표시 등) |

선을 제외한 도형은 `ShapeParams`로 모양을 정하며, 필드는 아래 표의 순서대로 지정합니다.

| 필드 | 기본값 | 의미 |
|---|---|---|
| `fill` | 흰색 | 채우기 색 |
| `outline` | 투명 | 외곽선 색 |
| `outlineThickness` | 0 | 외곽선 두께. 양수면 바깥쪽, 음수면 안쪽 |
| `origin` | `{0, 0}` | `position`에 놓일 점이자 회전 중심 |
| `rotation` | 0 | 회전 각도(도) |

`origin`의 기준은 도형마다 다릅니다. 사각형과 다각형은 로컬 원점(사각형은 왼쪽 위 꼭짓점)에서,
원과 부채꼴은 중심에서 잽니다. 사각형을 중심 기준으로 배치하고 회전하려면 `origin`을 크기의
절반으로 지정합니다.

```cpp
const meno::Vec2f size{40.f, 20.f};
renderer.drawRect({{320.f, 240.f}, size}, {.origin = size * 0.5f, .rotation = 30.f});
```

외곽선만 그리려면 `fill`을 `meno::colors::Transparent`로 두고 `outline`과 `outlineThickness`를
함께 지정합니다.

## 색

| 만드는 방법 | 예시 |
|---|---|
| 채널 값(0~255) | `meno::Color{255, 128, 0}`, 반투명은 `meno::Color{255, 128, 0, 128}` |
| 6자리 `0xRRGGBB` | `meno::Color::fromRgb(0x4FC3F7)` |
| 8자리 `0xRRGGBBAA` | `meno::Color::fromRgba(0x4FC3F780)` |
| 불투명도만 바꾸기 | `meno::colors::White.withAlpha(128)` |
| 미리 정의된 색 | `meno::colors::White`, `Black`, `Red`, `Yellow`, `Transparent` 등 |

## 텍스트

글자를 그리려면 폰트가 필요합니다. 폰트 파일은 `game.resources().loadFont()`로 한 번만 읽어 두고,
그릴 때마다 그 폰트를 넘깁니다.

```cpp
const auto font = game.resources().loadFont("assets/fonts/game.ttf"); // run() 전에 한 번
int score = 0;

play.setDrawHandler([&](meno::Renderer& renderer) {
    if (font) {
        renderer.drawText(*font, "점수: " + std::to_string(score), {16.f, 16.f},
                          {.characterSize = 24, .color = meno::colors::White});
    }
});
```

`loadFont()`는 실패하면 빈 `shared_ptr`을 돌려주므로, 사용하기 전에 `if (font)`로 확인합니다.
`TextParams`는 `characterSize`(기본 24), `color`, `origin`, `rotation`, `outlineThickness`,
`outlineColor` 순서로 지정합니다.

글자를 가운데 정렬하려면 `measureTextBounds()`로 글자가 실제로 차지하는 영역을 구한 뒤, 그 중심을
`origin`으로 지정합니다.

```cpp
meno::TextParams params{.characterSize = 48};
params.origin = renderer.measureTextBounds(*font, "GAME OVER", params).center();
renderer.drawText(*font, "GAME OVER", {320.f, 240.f}, params); // 화면 가운데
```

## 이미지

그림 파일은 `game.resources().loadTexture()`로 읽습니다. 같은 경로를 다시 요청하면 처음 읽은
텍스처를 그대로 돌려주므로, 여러 곳에서 사용해도 파일은 한 번만 읽힙니다.

```cpp
const auto player = game.resources().loadTexture("assets/player.png");

play.setDrawHandler([&](meno::Renderer& renderer) {
    if (player) {
        renderer.draw(*player, {.position = {100.f, 50.f}});
    }
});
```

`SpriteParams`는 `position`, `origin`, `scale`, `rotation`, `tint`, `source`, `flipX`,
`flipY` 순서로 지정합니다.

- `origin`은 텍스처 픽셀 단위의 기준점입니다. 기본값 `{0, 0}`이면 텍스처의 왼쪽 위 꼭짓점이
  `position`에 놓입니다.
- `tint`는 텍스처 색에 곱해지는 색입니다. 흰색이면 원본 그대로이며, `withAlpha()`로 불투명도를
  낮추면 반투명하게 그려집니다.
- 좌우를 뒤집을 때는 음수 `scale` 대신 `flipX`를 씁니다. 그러면 그려지는 위치가 바뀌지 않습니다.
- `source`로 텍스처의 일부만 그릴 수 있으며, 스프라이트 시트와 애니메이션에 씁니다.

픽셀 아트를 확대할 때 흐려지지 않게 하려면 보간을 끈 상태(기본값)로 둡니다. 부드럽게 확대하려면
`loadTexture(path, {.smooth = true})`로 읽습니다.

파일 없이 픽셀 배열로 텍스처를 만들 때는 `meno::Texture::fromPixels()`를 씁니다. 위 완성 코드의
체크무늬가 그 예입니다.

## 주의할 점

- `fromRgb()`에는 6자리만 넣습니다. 8자리를 넣으면 상수식에서는 컴파일 오류가 나고, 그 밖에서는
  눈에 띄도록 자홍색이 됩니다. 반투명한 색은 `withAlpha()`나 `fromRgba()`로 만듭니다.
- 외곽선 색 `outline`의 기본값은 투명입니다. 따라서 `outlineThickness`만 바꾸면 외곽선이 보이지
  않습니다.
- `drawPolygon()`은 볼록한 다각형만 올바르게 채웁니다. 오목한 모양은 외곽선만 그리거나, 여러 개의
  볼록한 조각으로 나눠 그립니다.
- 한글을 그리려면 한글 글리프가 있는 폰트를 씁니다(예: Windows의 `C:/Windows/Fonts/malgun.ttf`).
  폰트에 글리프가 없는 글자는 화면에 나타나지 않습니다.
- 파일은 `run()` 전이나 Scene을 준비할 때 한 번 읽어 두고, draw 핸들러에서는 그리기만 합니다.
- 옵션 구조체(`ShapeParams`, `TextParams`, `SpriteParams` 등)의 필드는 선언된 순서대로 지정해야
  컴파일됩니다.

## 관련 예제와 API

- 예제:
  [render_basics](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/render_basics/main.cpp)
  (그리기 API를 한 화면에서 모두 보여 줍니다)
- API: [Renderer](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_renderer.html),
  [ShapeParams](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_shape_params.html),
  [TextParams](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_text_params.html),
  [SpriteParams](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_sprite_params.html),
  [Color](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_color.html),
  [Texture](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_texture.html),
  [Font](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_font.html),
  [ResourceManager](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_resource_manager.html)

이전 장: [2장. 첫 게임: Game과 Scene](02-first-game.md) · 다음 장: [4장. 입력](04-input.md)
