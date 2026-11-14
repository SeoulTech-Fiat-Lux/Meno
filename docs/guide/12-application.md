# 12장. Game 없이 루프 직접 돌리기

`Game`은 창, 렌더러, 리소스 캐시, Scene을 묶어 루프의 흐름을 정해 둡니다. 이 장에서는 그 아래층인
`Application`을 직접 상속해 게임 루프의 각 단계를 손수 채웁니다. 이 방식은 Scene이 필요 없는 작은
도구나 실험용 프로그램을 만들 때, 또는 `Game`이 정한 흐름과 다르게 그려야 할 때 씁니다.

## 완성 코드

공 하나가 좌우로 오갑니다. 일부러 update를 초당 30회로 줄이고 그리기는 초당 120회까지 허용해,
보간으로 움직임을 부드럽게 만드는 방법을 보여 줍니다. Esc를 누르면 끝납니다.

```cpp
#include <meno/Meno.hpp>

#include <cmath>

class Demo final : public meno::Application {
public:
    Demo() : Application({.fixedTimeStep = 1.0 / 30.0, .framerateLimit = 120}) {}

private:
    meno::Window window_{{640u, 480u}, "Application"};
    meno::Renderer renderer_{window_}; // Window보다 뒤에 선언해야 Window가 먼저 만들어집니다.
    meno::Vec2f previous_{100.f, 240.f};
    meno::Vec2f current_{100.f, 240.f};
    float speed_ = 240.f;

    void processEvents() override {
        window_.pollEvents();
        if (!window_.isOpen() || window_.input().wasPressed(meno::Key::Escape)) {
            stop();
        }
    }

    void update(double fixedDeltaTime) override {
        previous_ = current_;
        current_.x += speed_ * static_cast<float>(fixedDeltaTime);
        if (current_.x > 540.f) {
            speed_ = -std::abs(speed_);
        } else if (current_.x < 100.f) {
            speed_ = std::abs(speed_);
        }
    }

    void render(double interpolationAlpha) override {
        // 직전 update와 마지막 update 사이를 alpha만큼 섞은 위치에 그립니다.
        const float alpha = static_cast<float>(interpolationAlpha);
        const meno::Vec2f drawn = previous_ + (current_ - previous_) * alpha;
        renderer_.beginFrame(meno::Color::fromRgb(0x1E2430));
        renderer_.drawCircle(drawn, 20.f, {.fill = meno::Color::fromRgb(0x00B1ED)});
        renderer_.endFrame();
    }
};

int main() {
    Demo demo;
    demo.run();
}
```

## Game과 Application

| | `Game` | `Application` |
|---|---|---|
| 창과 렌더러 | 만들어 둠 | 직접 만듦(`Window`, `Renderer`) |
| 리소스 캐시, Scene | 있음 | 필요하면 직접 둠 |
| 루프 단계 | 정해져 있어 재정의할 수 없음 | 각 단계를 재정의함 |
| 게임 규칙 | Scene의 핸들러에 붙임 | `update()`에 직접 씀 |

`Game`도 `Application`을 상속한 클래스입니다. 따라서 고정 시간 간격과 프레임 제한, 그리고 한 번에
하나만 실행할 수 있다는 규칙은 두 방식에 똑같이 적용됩니다.

## 루프의 각 단계

`run()`은 루프에 들어가기 전에 `onStart()`를 한 번 호출하고, 그 뒤 프레임마다 다음 함수를 차례로
호출합니다. `stop()`으로 루프가 끝나면 마지막으로 `onStop()`을 호출합니다.

| 함수 | 호출 시점 | 할 일 |
|---|---|---|
| `onStart()` | 루프 시작 전 한 번 | 에셋 로딩 같은 준비 |
| `processEvents()` | 프레임마다 한 번, update 전 | `pollEvents()`, 창이 닫히면 `stop()` |
| `update(fixedDeltaTime)` | 고정 시간 간격마다 0번 이상 | 게임 상태 갱신 |
| `render(alpha)` | 프레임마다 한 번, update 뒤 | `beginFrame()` → 그리기 → `endFrame()` |
| `onStop()` | `stop()`으로 루프가 끝난 뒤 한 번 | 정리 |

`onStart()`에서 걸린 시간은 게임 시간에 들어가지 않으므로, 로딩이 길어도 첫 프레임에 update가
한꺼번에 몰리지 않습니다. 창이 닫혔을 때 `stop()`을 호출하는 일도 직접 해야 합니다. 호출하지 않으면
창이 닫힌 뒤에도 루프가 계속 돕니다.

`render()`에서는 화면을 지우는 `beginFrame()`과 화면에 표시하는 `endFrame()`을 직접 호출합니다.
`beginFrame()`은 지금의 창 크기에 맞춰 카메라를 다시 적용하므로, 그보다 먼저 `pollEvents()`가 창
크기 변경을 반영해 두어야 합니다. 위 표의 순서대로 구현하면 이 조건은 저절로 지켜집니다.

## 보간으로 부드럽게 그리기

update는 고정 간격으로만 호출되므로, 그리기가 update보다 자주 일어나면 같은 위치를 여러 번 그리게
되어 움직임이 끊겨 보입니다. `render()`가 받는 `interpolationAlpha`는 마지막 update 이후 다음
update까지 진행된 비율(0 이상 1 미만)입니다. 위 예제처럼 update마다 직전 상태를 저장해 두고, 두 상태
사이를 alpha만큼 섞은 위치에 그리면 움직임이 부드러워집니다. 보간하지 않는다면 이 값은 무시해도
됩니다.

## 시간 읽기

`meno::Time`의 정적 함수로 어디서든 시간을 읽을 수 있습니다. 단위는 초입니다.

| 함수 | 값 |
|---|---|
| `Time::fixedDeltaTime()` | `ApplicationConfig::fixedTimeStep` |
| `Time::deltaTime()` | 현재 update의 시간 간격. update 안에서는 `fixedDeltaTime()`과 같음 |
| `Time::elapsedTime()` | 지금까지 진행된 update 시간의 합(게임 시간) |
| `Time::frameDeltaTime()` | 이번 프레임의 실제 시간. 최대 `fixedTimeStep × maxUpdatesPerFrame` |
| `Time::realElapsedTime()` | `onStart()`가 끝난 뒤 흐른 실제 시간의 합 |

게임 시간(`elapsedTime`)은 update가 진행된 만큼만 흐릅니다. 프레임이 크게 밀려 시간을 버리면 실제
시간(`realElapsedTime`)보다 느리게 흐르므로, 게임 규칙에는 게임 시간을 씁니다.

## 입력의 틱

`Application`에서도 4장의 "이번 틱" 규칙이 그대로 적용됩니다. `update()` 안에서는 update 한 번이 한
틱이고, `processEvents()`나 `render()`처럼 update 밖에서는 `pollEvents()`부터 다음
`pollEvents()`까지가 한 틱입니다. 위 예제는 Esc를 `processEvents()`에서 확인하는데, 이 규칙 덕분에
프레임마다 정확히 한 번만 판정됩니다.

## 가짜 시계로 테스트하기

`run(clock)`에 `Clock`을 넘기면 그 시계로 시간을 잽니다. 시각 함수와 대기 함수를 주입한 가짜 시계를
쓰면 실제로 기다리지 않고 원하는 만큼 시간을 진행시킬 수 있습니다. 루프의 동작을 자동 테스트로
확인할 때 유용합니다.

```cpp
meno::Clock::TimePoint now{};
meno::Clock clock{[&] { return now; }, [&](meno::Clock::Duration wait) { now += wait; }};
now += std::chrono::milliseconds(16);
const double seconds = clock.restart(); // 0.016
```

## 주의할 점

- `Renderer`는 자신을 만든 `Window`보다 먼저 사라져야 합니다. 멤버로 둘 때는 `Window`를 먼저
  선언합니다.
- 창이 닫혔을 때 `stop()`을 호출하지 않으면 루프가 끝나지 않습니다.
- 한 번에 하나의 `Application`(`Game` 포함)만 `run()`할 수 있습니다.
- `update()`에서 `Vec2f`에 시간을 곱할 때는 `static_cast<float>(fixedDeltaTime)`로 변환합니다.

## 관련 예제와 API

- 예제:
  [input_demo](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/v0.2.0/examples/input_demo/main.cpp)
  (`Application`을 직접 상속하고 보간해 그립니다)
- API: [Application](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_application.html),
  [ApplicationConfig](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_application_config.html),
  [Time](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_time.html),
  [Clock](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_clock.html),
  [Window](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_window.html),
  [Renderer](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_renderer.html)

이로써 사용자 설명서를 마칩니다. 각 클래스와 함수의 자세한 규칙은
[API 레퍼런스](https://seoultech-fiat-lux.github.io/Meno/topics.html)에서 모듈별로 찾을 수 있습니다.

이전 장: [11장. 스프라이트 애니메이션](11-animation.md)
