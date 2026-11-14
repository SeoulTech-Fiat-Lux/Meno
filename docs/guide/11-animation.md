# 11장. 스프라이트 애니메이션

이 장에서는 프레임 여러 장을 이어 붙인 스프라이트 시트에서 한 프레임씩 골라 그리는 방식으로
애니메이션을 만듭니다.

## 완성 코드

← → 키로 캐릭터를 움직입니다. 움직이는 동안에만 걷는 동작의 프레임이 바뀌고, 왼쪽으로 갈 때는 그림을
뒤집습니다. 스프라이트 시트는 파일 대신 코드로 만듭니다.

```cpp
#include <meno/Meno.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

constexpr int frameSize = 16; // 프레임 한 장의 크기(픽셀)
constexpr int frameCount = 4;

// 16×16 프레임 네 장을 가로로 이은 64×16 스프라이트 시트를 만듭니다. 몸통은 그대로 두고, 아래쪽의
// 흰 "다리"가 프레임마다 다른 위치에 그려집니다.
std::shared_ptr<const meno::Texture> makeSheet() {
    const meno::Vec2u size{frameSize * frameCount, frameSize};
    std::vector<std::uint8_t> pixels(std::size_t{size.x} * size.y * 4);
    for (unsigned int y = 0; y < size.y; ++y) {
        for (unsigned int x = 0; x < size.x; ++x) {
            const unsigned int frame = x / frameSize;
            const unsigned int local = x % frameSize;
            const bool body = y < 12 && local >= 2 && local < 14;
            const bool leg = y >= 12 && local / 4 == frame;
            const meno::Color color = body  ? meno::Color::fromRgb(0x00B1ED)
                                      : leg ? meno::colors::White
                                            : meno::colors::Transparent;
            const std::size_t i = (std::size_t{y} * size.x + x) * 4;
            pixels[i] = color.r;
            pixels[i + 1] = color.g;
            pixels[i + 2] = color.b;
            pixels[i + 3] = color.a;
        }
    }
    auto texture = meno::Texture::fromPixels(size, pixels.data());
    return texture ? std::make_shared<const meno::Texture>(std::move(*texture)) : nullptr;
}

int main() {
    meno::Game game{{.application = {.framerateLimit = 60},
                     .windowSize = {640, 480},
                     .title = "Animation",
                     .clearColor = meno::Color::fromRgb(0x1E2430)}};
    meno::Scene& scene = game.scenes().createScene();

    meno::GameObject& hero = scene.createGameObject();
    hero.transform().position = {288.f, 208.f};
    hero.transform().scale = 4.f;
    hero.sprite().setTexture(makeSheet());
    hero.sprite().source = meno::Recti{0, 0, frameSize, frameSize};

    const double frameTime = 0.12; // 프레임 한 장을 보여 주는 시간(초)
    double timer = 0.0;
    int frame = 0;

    scene.setUpdateHandler([&](double fixedDeltaTime) {
        const meno::Input& input = game.window().input();
        float direction = 0.f;
        if (input.isDown(meno::Key::Left)) {
            direction -= 1.f;
        }
        if (input.isDown(meno::Key::Right)) {
            direction += 1.f;
        }
        hero.transform().position.x += direction * 160.f * static_cast<float>(fixedDeltaTime);

        if (direction != 0.f) {
            hero.sprite().flipX = direction < 0.f;
            timer += fixedDeltaTime;
            while (timer >= frameTime) {
                timer -= frameTime;
                frame = (frame + 1) % frameCount;
            }
        } else {
            timer = 0.0;
            frame = 0; // 멈추면 첫 프레임으로 돌아갑니다.
        }
        hero.sprite().source = meno::Recti{frame * frameSize, 0, frameSize, frameSize};
    });

    (void)game.scenes().changeScene(scene.id());
    game.run();
}
```

## 스프라이트 시트와 source

스프라이트 시트는 애니메이션의 프레임을 이미지 하나에 나란히 모은 것입니다. 프레임을 파일마다 따로
두지 않으므로 텍스처를 한 번만 읽으면 됩니다.

시트에서 한 프레임만 그리려면 `source`에 그 프레임의 영역을 **텍스처 픽셀 좌표**로 지정합니다.
`source`는 `Recti{x, y, 너비, 높이}`이며, 비어 있으면 텍스처 전체를 그립니다.

- GameObject에서는 `object.sprite().source`를 바꿉니다.
- GameObject 없이 그릴 때는 `renderer.draw(texture, {.source = ...})`처럼 `SpriteParams::source`를
  씁니다.

프레임이 가로 한 줄로 놓여 있다면 i번째 프레임(0부터 셈)은 `Recti{i * 너비, 0, 너비, 높이}`입니다.
여러 줄로 놓여 있다면 행과 열을 계산해 `x`와 `y`를 정합니다.

## 프레임을 넘기는 시점

프레임은 update 핸들러에서 시간을 누적해 넘깁니다. update 핸들러가 받는 `fixedDeltaTime`은 항상 같은
값이므로, 위 예제처럼 누적한 시간이 프레임 한 장의 표시 시간을 넘을 때마다 다음 프레임으로 넘기면
컴퓨터 성능과 관계없이 같은 속도로 재생됩니다.

`while`로 넘기는 이유는 표시 시간이 update 간격보다 짧은 경우에도 프레임을 빠뜨리지 않기
위해서입니다. 멈췄을 때 첫 프레임으로 되돌리는 처리나, 공격 동작처럼 한 번만 재생하고 멈추는
애니메이션도 같은 방식에 조건만 더하면 만들 수 있습니다.

## 방향 바꾸기

왼쪽을 볼 때 그림을 뒤집으려면 `sprite().flipX`를 true로 둡니다. 음수 배율과 달리 그려지는 위치가
바뀌지 않으므로, 뒤집어도 캐릭터가 제자리에 있습니다. 위아래는 `flipY`로 뒤집습니다.

## 주의할 점

- `source`는 배율을 적용하기 전의 텍스처 픽셀 좌표입니다. `transform().scale`로 키운 크기를 넣지
  않습니다.
- 픽셀 아트 시트는 보간을 끈 상태(기본값)로 둡니다. 보간을 켜면 확대할 때 이웃한 프레임의 색이
  가장자리에 번질 수 있습니다.
- `Sprite::origin`과 `SpriteParams::origin`은 `source`로 고른 영역의 왼쪽 위를 기준으로 잽니다.

## 관련 예제와 API

- API: [Sprite](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_sprite.html),
  [SpriteParams](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_sprite_params.html),
  [Rect](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_rect.html)

이전 장: [10장. 카메라와 좌표](10-camera.md) · 다음 장: [12장. Game 없이 루프 직접 돌리기](12-application.md)
