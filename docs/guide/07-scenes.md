# 7장. 씬 전환과 씬 상태

이 장에서는 타이틀, 게임, 결과 화면을 각각 Scene으로 만들고 서로 전환합니다. Scene마다 상태를 맡기는
방법과 게임을 처음부터 다시 시작하는 방법도 다룹니다.

## 완성 코드

타이틀에서 Enter를 누르면 게임이 시작되어 10초 동안 Space를 누른 횟수를 셉니다. 시간이 다 되면
결과를 보여 주고, 결과 화면에서 Enter를 누르면 타이틀로 돌아갑니다. 폰트가 없으면 글자 없이 배경색과
시간 막대만 보입니다.

```cpp
#include <meno/Meno.hpp>

#include <algorithm>
#include <memory>
#include <string>

// 게임 Scene에 맡길 상태입니다.
struct PlayState {
    int score = 0;
    double timeLeft = 10.0;
};

int main() {
    meno::Game game{
        {.application = {.framerateLimit = 60}, .windowSize = {640, 480}, .title = "Scenes"}};
    const auto font = game.resources().loadFont("assets/fonts/game.ttf");

    meno::SceneManager& scenes = game.scenes();
    meno::Scene& title = scenes.createScene();
    meno::Scene& play = scenes.createScene();
    meno::Scene& result = scenes.createScene();
    play.setState(std::make_unique<PlayState>());

    // 배경을 칠하고, 폰트가 있으면 화면 가운데에 글자를 씁니다.
    const auto background = [](meno::Renderer& renderer, meno::Color color) {
        renderer.drawRect({{0.f, 0.f}, {640.f, 480.f}}, {.fill = color});
    };
    const auto caption = [&](meno::Renderer& renderer, const std::string& text, float y) {
        if (font) {
            meno::TextParams params{.characterSize = 32};
            params.origin = renderer.measureTextBounds(*font, text, params).center();
            renderer.drawText(*font, text, {320.f, y}, params);
        }
    };
    const auto enterPressed = [&] { return game.window().input().wasPressed(meno::Key::Enter); };

    // 타이틀: Enter를 누르면 게임을 시작합니다.
    title.setUpdateHandler([&](double) {
        if (enterPressed()) {
            (void)scenes.changeScene(play.id()); // 이 update가 끝난 직후에 바뀝니다.
        }
    });
    title.setDrawHandler([&](meno::Renderer& renderer) {
        background(renderer, meno::Color::fromRgb(0x1E3A5F));
        caption(renderer, "Press Enter", 240.f);
    });

    // 게임: 들어올 때마다 상태를 초기화하므로, 다시 시작해도 처음부터 진행됩니다.
    play.setEnterHandler([&] { *play.getState<PlayState>() = PlayState{}; });
    play.setUpdateHandler([&](double fixedDeltaTime) {
        PlayState& state = *play.getState<PlayState>();
        if (game.window().input().wasPressed(meno::Key::Space)) {
            ++state.score;
        }
        state.timeLeft -= fixedDeltaTime;
        if (state.timeLeft <= 0.0) {
            (void)scenes.changeScene(result.id());
        }
    });
    play.setDrawHandler([&](meno::Renderer& renderer) {
        const PlayState& state = *play.getState<PlayState>();
        background(renderer, meno::Color::fromRgb(0x1E2430));
        const float ratio = static_cast<float>(std::max(state.timeLeft, 0.0) / 10.0);
        renderer.drawRect({{40.f, 400.f}, {560.f * ratio, 16.f}},
                          {.fill = meno::Color::fromRgb(0x00B1ED)}); // 남은 시간
        caption(renderer, std::to_string(state.score), 220.f);
    });

    // 결과: 게임 Scene의 상태에서 점수를 읽어 보여 줍니다.
    result.setUpdateHandler([&](double) {
        if (enterPressed()) {
            (void)scenes.changeScene(title.id());
        }
    });
    result.setDrawHandler([&](meno::Renderer& renderer) {
        background(renderer, meno::Color::fromRgb(0x5F1E2A));
        caption(renderer, "Score: " + std::to_string(play.getState<PlayState>()->score), 220.f);
        caption(renderer, "Press Enter", 280.f);
    });

    (void)scenes.changeScene(title.id());
    game.run();
}
```

## Scene을 여러 개 두기

Scene은 타이틀, 게임, 결과처럼 화면 단위로 하나씩 만듭니다. update 핸들러가 호출되고 화면에 그려지는
Scene은 활성 Scene뿐이며, 나머지 Scene은 멈춰 있습니다. 멈춰 있는 Scene도 자신의 GameObject와 상태를
그대로 가지고 있으므로, 다시 활성화하면 떠날 때의 상태에서 이어집니다.

`createScene()`이 돌려준 참조는 그 Scene을 지우거나 게임이 끝날 때까지 유효합니다. 위 예제처럼 지역
변수에 받아 두고 핸들러에서 써도 됩니다.

## 전환 시점

`scenes.changeScene(id)`는 호출한 곳에 따라 전환되는 시점이 다릅니다.

| 호출한 곳 | 전환 시점 |
|---|---|
| 핸들러 밖(`run()` 전 등) | 바로 |
| update 핸들러 안 | 그 update가 끝난 직후 |
| enter·exit·draw 핸들러 안 | 다음 update가 시작될 때 |

전환할 때는 이전 Scene의 exit 핸들러가 먼저, 새 Scene의 enter 핸들러가 나중에 호출됩니다. 핸들러
안에서 전환을 여러 번 요청하면 마지막 요청만 반영됩니다.

핸들러 안에서 `changeScene()`을 호출해도 그 핸들러는 끝까지 실행됩니다. 위 예제에서 시간이 다 되어
결과 Scene으로 전환을 요청한 update도, 요청 뒤에 남은 코드를 모두 실행한 다음에 끝납니다.

## enter와 exit 핸들러

enter 핸들러는 Scene이 활성화될 때마다 호출됩니다. 위 예제처럼 여기서 점수와 타이머를 초기화하면,
결과 화면에서 다시 시작해도 게임이 처음부터 진행됩니다.

exit 핸들러는 다른 Scene으로 바뀔 때와 게임이 끝날 때 호출됩니다. 여기서는 배경음악을 멈추는 것처럼
그 Scene에서 시작한 일을 정리합니다. exit 핸들러가 예외를 던지면 전환이 취소되어 이전 Scene이 그대로
활성 상태로 남습니다.

## Scene 상태

여러 Scene에 걸친 상태는 1부처럼 `main`의 지역 변수나 `Game`을 상속한 클래스의 멤버로 둘 수
있습니다. 특정 Scene에만 속한 상태는 `setState()`로 그 Scene에 맡길 수도 있습니다.

```cpp
struct PlayState {
    int score = 0;
};

play.setState(std::make_unique<PlayState>());

if (PlayState* state = play.getState<PlayState>()) {
    state->score += 10;
}
```

- `setState()`는 상태 객체의 소유권을 Scene으로 옮깁니다. 상태는 Scene이 사라질 때 함께 파괴되며,
  Scene을 전환하는 것만으로는 사라지지 않습니다.
- `getState<T>()`는 등록한 상태를 포인터로 돌려줍니다. 상태를 등록하지 않았거나 T가 등록할 때의
  타입과 다르면 `nullptr`을 돌려줍니다.
- 한 Scene에는 상태를 하나만 등록할 수 있습니다. 상태가 이미 등록된 Scene의 `setState()`를 다시
  호출하면 `std::logic_error` 예외가 발생합니다. 상태를 초기화하려면 위 예제처럼 등록된 객체에 새
  값을 대입합니다.

## Scene 지우기

더는 쓰지 않는 Scene은 `scenes.deleteScene(id)`로 지웁니다. 그 Scene의 GameObject와 상태도 함께
파괴됩니다. 단, 활성 Scene의 ID를 넘기거나 Scene의 핸들러 안에서 호출하면 지우지 않고 `false`를
돌려줍니다. 게임이 끝날 때는 `Game`이 남은 Scene을 모두 정리하므로 따로 지울 필요가 없습니다.

## 주의할 점

- 핸들러 안에서 요청한 전환은 바로 일어나지 않습니다. 요청한 뒤에도 그 핸들러는 이전 Scene에서 계속
  실행됩니다.
- `wasPressed()`는 한 틱에서만 true이므로, 타이틀에서 누른 Enter가 바로 다음 Scene의 update에서 다시
  감지되지 않습니다.
- 활성 Scene은 지울 수 없습니다. 다른 Scene으로 전환한 뒤에 지웁니다.
- `getState<T>()`는 실패하면 `nullptr`을 돌려줍니다. 상태를 등록했는지 확실하지 않다면 결과를 확인한
  뒤 씁니다.

## 관련 예제와 API

- 예제:
  [scene_transition](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/scene_transition/main.cpp)
  (Scene 두 개를 3초마다 전환합니다)
- API: [SceneManager](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_scene_manager.html),
  [Scene](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_scene.html)

이전 장: [6장. 충돌 판정](06-collision.md) · 다음 장: [8장. 오디오](08-audio.md)
