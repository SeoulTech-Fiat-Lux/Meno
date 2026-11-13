# 4장. 입력

이 장에서는 키보드와 마우스 입력을 읽는 방법을 익히고, 마지막으로 지금까지 배운 내용을 모아
퐁(Pong)을 만듭니다.

## 입력 읽기

입력은 `game.window().input()`으로 읽습니다. 창이 프레임마다 입력 상태를 갱신하므로 이벤트를 직접
처리할 필요가 없습니다. 게임 규칙에 쓰는 입력은 update 핸들러에서 읽습니다.

```cpp
play.setUpdateHandler([&](double fixedDeltaTime) {
    const meno::Input& input = game.window().input();
    const float step = 300.f * static_cast<float>(fixedDeltaTime);
    if (input.isDown(meno::Key::Right)) { // 누르고 있는 동안 계속
        position.x += step;
    }
    if (input.wasPressed(meno::Key::Space)) { // 누른 순간 한 번만
        velocity.y = -400.f;
    }
    if (input.wasPressed(meno::Key::Escape)) {
        game.stop(); // 게임을 끝냅니다.
    }
});
```

| 함수 | true를 돌려주는 때 |
|---|---|
| `isDown(key)` | 눌려 있는 동안 계속 |
| `wasPressed(key)` | 누른 순간 한 번 |
| `wasReleased(key)` | 뗀 순간 한 번 |

세 함수 모두 `meno::Key`와 `meno::MouseButton`을 인자로 받을 수 있습니다. 키를 누르고 있을 때
운영체제가 보내는 반복 입력은 무시하므로, `wasPressed()`는 한 번 누를 때마다 한 번만 true가 됩니다.

## "이번 틱" 규칙

`wasPressed()`, `wasReleased()`, `mouseWheelDelta()`, `text()`는 **이번 틱**에 일어난 일만 알려
줍니다. 동작 이름으로 묻는 `wasActionPressed()`와 `wasActionReleased()`도 마찬가지입니다. update
핸들러 안에서는 update 한 번이 한 틱입니다. 한 프레임에서 update가 여러 번 실행되더라도, 한 번 누른
키는 그 뒤 처음 실행되는 update에서만 `wasPressed()`가 true입니다. 따라서 점프가 두 번 일어나거나
입력을 놓치는 일이 없습니다.

draw 핸들러처럼 update 밖에서 읽을 때는 한 프레임이 한 틱입니다. 그러므로 게임 규칙은 update
핸들러에서 처리합니다.

## 마우스

```cpp
const meno::Input& input = game.window().input();
if (input.wasPressed(meno::MouseButton::Left)) {
    const meno::Vec2f clicked{input.mousePosition()}; // 창 왼쪽 위 기준 픽셀
}
const float wheel = input.mouseWheelDelta(); // 이번 틱에 굴린 양. 위로 굴리면 양수
```

`mousePosition()`은 정수 벡터(`Vec2i`)를 돌려주므로, `Vec2f`와 함께 계산하려면 위처럼 변환합니다. 이
좌표는 창을 기준으로 하므로 카메라를 쓰면 월드 좌표와 달라지며, 이때는
`game.renderer().screenToWorld()`로 변환합니다([10장](10-camera.md)).

## 동작 이름으로 묶기

한 동작에 키를 여러 개 쓰려면 "jump" 같은 동작 이름에 키를 묶습니다. 묶인 키 중 하나라도 눌려 있으면
그 동작이 눌린 것으로 봅니다.

```cpp
meno::Input& input = game.window().input();
input.bind("jump", meno::Key::Space);
input.bind("jump", meno::Key::W);
input.bind("jump", meno::MouseButton::Left);

play.setUpdateHandler([&](double) {
    if (game.window().input().wasActionPressed("jump")) {
        // Space를 누른 채 W를 더 눌러도 한 번만 들어옵니다.
    }
});
```

`isActionDown()`, `wasActionPressed()`, `wasActionReleased()`는 각각 `isDown()`, `wasPressed()`,
`wasReleased()`에 해당합니다. `unbind("jump")`는 그 동작에 묶인 키를 모두 풉니다. 키 하나만 따로 풀
수는 없으므로, 묶인 키를 바꾸려면 `unbind()`로 푼 뒤 다시 `bind()`합니다.

## 글자 입력

이름 입력처럼 글자를 입력받을 때는 `text()`를 씁니다. `text()`는 이번 틱에 입력된 글자를 UTF-8
문자열로 돌려주며, 한글은 조합이 끝난 글자만 들어옵니다.

```cpp
std::string name;

play.setUpdateHandler([&](double) {
    const meno::Input& input = game.window().input();
    name += input.text();
    if (input.wasPressed(meno::Key::Backspace) && !name.empty()) {
        // UTF-8에서 한 글자는 여러 바이트일 수 있으므로, 뒤따르는 바이트(10xxxxxx)까지 지웁니다.
        while (!name.empty() && (static_cast<unsigned char>(name.back()) & 0xC0) == 0x80) {
            name.pop_back();
        }
        if (!name.empty()) {
            name.pop_back();
        }
    }
});
```

백스페이스, 엔터, 탭 같은 제어 문자는 `text()`에 들어오지 않으므로, 위처럼 `wasPressed()`로 따로
처리합니다.

## 완성 코드: 퐁

왼쪽 막대는 W·S 키로, 오른쪽 막대는 ↑·↓ 키로 움직입니다. 공이 막대에 닿으면 튕겨 나가고, 화면의
왼쪽이나 오른쪽 끝을 지나가면 상대가 1점을 얻습니다. 충돌은 두 사각형이 겹치는지 검사하는
`Rectf::intersects()`로 판정합니다.

```cpp
#include <meno/Meno.hpp>

#include <algorithm>
#include <cmath>
#include <string>

int main() {
    meno::Game game{{.application = {.framerateLimit = 60},
                     .windowSize = {800, 600},
                     .title = "Pong",
                     .clearColor = meno::Color::fromRgb(0x0D1117)}};
    const meno::Vec2f field{game.window().size()};
    const auto font = game.resources().loadFont("assets/fonts/game.ttf");

    const meno::Vec2f paddleSize{16.f, 96.f};
    const float paddleSpeed = 420.f;
    const float ballRadius = 8.f;
    meno::Rectf left{{40.f, field.y / 2.f - paddleSize.y / 2.f}, paddleSize};
    meno::Rectf right{{field.x - 40.f - paddleSize.x, left.position.y}, paddleSize};
    meno::Vec2f ball = field * 0.5f;
    meno::Vec2f velocity{360.f, 180.f};
    int leftScore = 0;
    int rightScore = 0;

    const auto serve = [&](float direction) {
        ball = field * 0.5f;
        velocity = {360.f * direction, 180.f};
    };

    meno::Scene& play = game.scenes().createScene();
    play.setUpdateHandler([&](double fixedDeltaTime) {
        const meno::Input& input = game.window().input();
        const float dt = static_cast<float>(fixedDeltaTime);
        if (input.wasPressed(meno::Key::Escape)) {
            game.stop();
        }

        // 막대를 움직이고, 화면 밖으로 나가지 않게 위치를 제한합니다.
        const auto move = [&](meno::Rectf& paddle, meno::Key up, meno::Key down) {
            if (input.isDown(up)) {
                paddle.position.y -= paddleSpeed * dt;
            }
            if (input.isDown(down)) {
                paddle.position.y += paddleSpeed * dt;
            }
            paddle.position.y = std::clamp(paddle.position.y, 0.f, field.y - paddle.size.y);
        };
        move(left, meno::Key::W, meno::Key::S);
        move(right, meno::Key::Up, meno::Key::Down);

        ball += velocity * dt;
        if (ball.y < ballRadius || ball.y > field.y - ballRadius) {
            ball.y = std::clamp(ball.y, ballRadius, field.y - ballRadius);
            velocity.y = -velocity.y;
        }

        // 공을 감싸는 사각형이 막대와 겹치면, 그 막대에서 멀어지는 쪽으로 튕깁니다.
        const meno::Rectf ballBox{ball - meno::Vec2f{ballRadius, ballRadius},
                                  {ballRadius * 2.f, ballRadius * 2.f}};
        if (ballBox.intersects(left)) {
            velocity.x = std::abs(velocity.x);
        } else if (ballBox.intersects(right)) {
            velocity.x = -std::abs(velocity.x);
        }

        if (ball.x < 0.f) {
            ++rightScore;
            serve(1.f);
        } else if (ball.x > field.x) {
            ++leftScore;
            serve(-1.f);
        }
    });
    play.setDrawHandler([&](meno::Renderer& renderer) {
        renderer.drawLine({field.x / 2.f, 0.f}, {field.x / 2.f, field.y},
                          {.color = meno::colors::Gray, .thickness = 2.f});
        renderer.drawRect(left);
        renderer.drawRect(right);
        renderer.drawCircle(ball, ballRadius, {.fill = meno::Color::fromRgb(0x00B1ED)});
        if (font) {
            const std::string score =
                std::to_string(leftScore) + "   " + std::to_string(rightScore);
            meno::TextParams params{.characterSize = 40};
            params.origin = renderer.measureTextBounds(*font, score, params).center();
            renderer.drawText(*font, score, {field.x / 2.f, 40.f}, params);
        }
    });

    (void)game.scenes().changeScene(play.id());
    game.run();
}
```

점수는 `assets/fonts/game.ttf`가 있을 때만 표시됩니다. 가지고 있는 TTF 폰트 파일을 이 이름으로
복사해 두면 됩니다.

이로써 1부를 마칩니다. 게임 루프와 그리기, 입력, 사각형 충돌 판정만으로도 간단한 게임 하나를
처음부터 끝까지 만들 수 있습니다.

## 주의할 점

- 키 이름은 자판에 적힌 글자를 따릅니다. 예를 들어 AZERTY 자판에서 QWERTY의 Q 위치에 있는 키를
  누르면 `Key::A`가 들어옵니다.
- `Num0`~`Num9`는 자판 위쪽 줄의 숫자 키이고, `Numpad0`~`Numpad9`는 숫자 패드의 키입니다.
- 창이 포커스를 잃으면 눌려 있던 키와 버튼은 모두 뗀 것으로 처리되며, 그 틱에서 `wasReleased()`가
  true를 돌려줍니다.
- `Rectf::intersects()`는 경계만 맞닿은 경우를 겹치지 않은 것으로 봅니다.

## 관련 예제와 API

- 예제:
  [input_demo](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/input_demo/main.cpp)
  (키보드, 마우스, 동작 묶기를 한 화면에서 보여 줍니다)
- API: [Input](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_input.html), [입력
  모듈](https://seoultech-fiat-lux.github.io/Meno/group__input.html)(`Key`, `MouseButton`),
  [Rect](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_rect.html),
  [Vec2](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_vec2.html)

이전 장: [3장. 그리기](03-drawing.md) · 다음 장: [5장. GameObject와 컴포넌트](05-gameobjects.md)
