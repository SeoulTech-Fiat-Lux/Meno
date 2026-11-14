# 2장. 첫 게임: Game과 Scene

이 장에서는 화면 안을 튕겨 다니는 공을 만들면서 meno 게임의 기본 구조를 익힙니다. `Game`은 창과 게임
루프를, `Scene`에 붙인 핸들러는 게임 규칙을 담당합니다.

## 완성 코드

```cpp
#include <meno/Meno.hpp>

#include <cmath>

int main() {
    meno::Game game{{.application = {.framerateLimit = 60},
                     .windowSize = {640, 480},
                     .title = "Bouncing ball",
                     .clearColor = meno::Color::fromRgb(0x1E2430)}};

    const float radius = 16.f;
    meno::Vec2f position{320.f, 240.f};
    meno::Vec2f velocity{180.f, 120.f}; // 초당 이동 거리(픽셀)

    meno::Scene& play = game.scenes().createScene();
    play.setUpdateHandler([&](double fixedDeltaTime) {
        position += velocity * static_cast<float>(fixedDeltaTime);

        // 벽을 넘으면 안쪽으로 되돌리고, 그 축의 이동 방향을 바꿉니다.
        const meno::Vec2f size{game.window().size()};
        if (position.x < radius) {
            position.x = radius;
            velocity.x = std::abs(velocity.x);
        } else if (position.x > size.x - radius) {
            position.x = size.x - radius;
            velocity.x = -std::abs(velocity.x);
        }
        if (position.y < radius) {
            position.y = radius;
            velocity.y = std::abs(velocity.y);
        } else if (position.y > size.y - radius) {
            position.y = size.y - radius;
            velocity.y = -std::abs(velocity.y);
        }
    });
    play.setDrawHandler([&](meno::Renderer& renderer) {
        renderer.drawCircle(position, radius, {.fill = meno::Color::fromRgb(0x00B1ED)});
    });

    (void)game.scenes().changeScene(play.id());
    game.run();
}
```

## Game: 창과 게임 루프

`meno::Game`은 창(`Window`), 렌더러(`Renderer`), 리소스 캐시(`ResourceManager`), 씬
관리자(`SceneManager`)를 한곳에 묶은 클래스입니다. `Game` 객체를 만드는 즉시 창이 뜨고, `run()`을
호출하면 창이 닫힐 때까지 게임 루프를 실행합니다. 네 구성 요소에는 각각 `game.window()`,
`game.renderer()`, `game.resources()`, `game.scenes()`로 접근합니다.

설정은 `GameConfig`로 넘깁니다.

| 필드 | 기본값 | 의미 |
|---|---|---|
| `application` | `{}` | 게임 루프의 시간 설정(`ApplicationConfig`) |
| `windowSize` | `{800, 600}` | 창 내부(그리기 영역)의 픽셀 크기 |
| `title` | `"meno"` | 창 제목 |
| `clearColor` | 검은색 | 프레임마다 화면을 지울 배경색 |

지정 초기화(`.windowSize = ...`)에서는 필드를 위 표의 순서대로 써야 하며, 순서가 바뀌면 컴파일되지
않습니다. 기본값을 쓸 필드는 생략해도 됩니다.

## Scene과 핸들러

게임 규칙은 `Scene`에 핸들러(람다)로 붙입니다. Scene은 타이틀, 게임, 게임 오버처럼 화면 단위로
만들며, 한 번에 하나만 활성 상태가 됩니다.

| 핸들러 | 호출 시점 | 주로 하는 일 |
|---|---|---|
| `setEnterHandler` | 이 Scene이 활성화될 때 | 점수 초기화 같은 준비 |
| `setUpdateHandler` | 고정 시간 간격마다(기본 초당 60회) | 게임 상태 갱신 |
| `setDrawHandler` | 프레임마다, Scene의 GameObject를 그린 뒤 | 그리기 |
| `setExitHandler` | 다른 Scene으로 바뀌거나 게임이 끝날 때 | 정리 |

Scene은 `game.scenes().createScene()`으로 만들고 `changeScene(id)`로 활성화합니다. 활성 Scene이
없으면 화면은 배경색으로만 칠해집니다. `changeScene()`은 존재하지 않는 ID를 받으면 `false`를
돌려줍니다. 반환값을 쓰지 않으면 컴파일러가 경고하므로, 확인할 필요가 없을 때는 `(void)`로
명시적으로 버립니다.

## 한 프레임에 일어나는 일

`run()`은 프레임마다 다음 과정을 되풀이합니다.

1. 창 이벤트와 입력을 처리합니다. 창이 닫혔으면 루프를 멈춥니다.
2. 흐른 시간에 맞춰 update 핸들러를 0번 이상 호출합니다.
3. 화면을 `clearColor`로 지우고 활성 Scene을 그립니다. GameObject를 먼저 그린 다음 draw 핸들러를
   호출합니다.

update 핸들러는 **고정 시간 간격**으로 호출됩니다. 인자 `fixedDeltaTime`은 항상 같은 값(기본
1/60초)입니다. 따라서 "초당 180픽셀"처럼 초 단위로 정한 속도에 이 값을 곱하면, 컴퓨터 성능과
관계없이 게임이 같은 속도로 진행됩니다. 프레임이 느려지면 한 프레임에서 update를 여러 번 호출해 밀린
시간을 따라잡고, 프레임이 빠르면 update 없이 그리기만 하는 프레임도 생깁니다. 밀린 update가
`maxUpdatesPerFrame`(기본 8회)을 넘을 만큼 느려지면 남은 시간은 버립니다. 덕분에 게임은 멈추지 않고
느려지기만 합니다.

draw 핸들러는 프레임마다 한 번 호출되는데, 초당 프레임 수는 컴퓨터와 상황에 따라 달라집니다. 그래서
게임 상태는 update 핸들러에서만 바꾸고, draw 핸들러에서는 그리기만 합니다.

`.application = {.framerateLimit = 60}`은 초당 그리는 횟수를 60회로 제한합니다. 기본값 0은 제한이
없다는 뜻이어서, 화면이 바뀌지 않아도 쉬지 않고 다시 그리느라 CPU를 많이 씁니다. 이 값은 그리는
횟수에만 영향을 주며 update 간격은 바꾸지 않습니다.

## 게임 상태를 둘 곳

위 예제는 `position` 같은 상태를 `main`의 지역 변수로 두고, 람다가 이를 참조(`[&]`)로 캡처합니다. 이
지역 변수들은 `game.run()`이 끝날 때까지 살아 있으므로 안전합니다. 반대로 `run()`보다 먼저 끝나는
함수의 지역 변수를 참조로 캡처하면, 핸들러가 호출될 때 그 변수는 이미 사라진 뒤입니다.

게임이 커지면 `Game`을 상속하고 상태를 멤버 변수로 두는 편이 관리하기 쉽습니다. 이때 Scene은 루프가
시작되기 직전에 한 번 호출되는 `onStart()`에서 구성합니다.

```cpp
#include <meno/Meno.hpp>

class BouncingBall final : public meno::Game {
public:
    BouncingBall() : Game({.application = {.framerateLimit = 60}, .title = "Bouncing ball"}) {}

private:
    meno::Vec2f position_{320.f, 240.f};
    meno::Vec2f velocity_{180.f, 120.f};

    void onStart() override {
        meno::Scene& play = scenes().createScene();
        play.setUpdateHandler([this](double fixedDeltaTime) {
            position_ += velocity_ * static_cast<float>(fixedDeltaTime);
        });
        play.setDrawHandler([this](meno::Renderer& renderer) {
            renderer.drawCircle(position_, 16.f, {.fill = meno::colors::White});
        });
        (void)scenes().changeScene(play.id());
    }
};

int main() {
    BouncingBall game;
    game.run();
}
```

Scene마다 따로 둘 상태는 `Scene::setState()`로 Scene에 맡길 수도 있습니다. 이 방법은
[7장](07-scenes.md)에서 설명합니다.

## 게임 끝내기

창의 닫기 버튼을 누르거나 핸들러 안에서 `game.stop()`을 호출하면 루프가 끝납니다. 이때 활성 Scene의
exit 핸들러가 호출된 뒤 `run()`이 끝납니다. Esc 키로 게임을 끝내는 방법은 [4장](04-input.md)에서
다룹니다.

## 주의할 점

- `Vec2f`에 `double`을 곱하면 컴파일되지 않습니다. `fixedDeltaTime`은
  `static_cast<float>(fixedDeltaTime)`로 변환한 뒤 곱합니다.
- 게임 상태는 update 핸들러에서 바꾸고, draw 핸들러에서는 그리기만 합니다.
- 한 번에 하나의 `Game`만 `run()`할 수 있습니다. 이미 실행 중일 때 다시 호출하면 `std::logic_error`
  예외가 발생합니다.
- `framerateLimit`을 정하지 않으면 그리는 횟수에 제한이 없어 CPU를 많이 씁니다.

## 관련 예제와 API

- 예제:
  [scene_transition](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/scene_transition/main.cpp)
  (Game을 상속하고, Scene 두 개를 3초마다 전환합니다)
- API: [Game](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_game.html),
  [GameConfig](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_game_config.html),
  [Scene](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_scene.html),
  [SceneManager](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_scene_manager.html),
  [ApplicationConfig](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_application_config.html)

이전 장: [1장. 설치와 첫 빌드](01-setup.md) · 다음 장: [3장. 그리기](03-drawing.md)
