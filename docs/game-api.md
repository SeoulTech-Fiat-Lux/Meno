# Game 통합 실행 경로

`Game`은 일반적인 Meno 게임에서 반복되던 `Window`, `Renderer`,
`ResourceManager`, `SceneManager` 연결과 `Application` 콜백을 한곳에 묶는다.
사용자는 `Game`을 상속하고 `onStart()`에서 장면을 구성하면 된다.

```cpp
#include <meno/Meno.hpp>

class MyGame final : public meno::Game {
public:
    MyGame()
        : Game({.application = {.framerateLimit = 60},
                .windowSize = {960u, 540u},
                .title = "My game",
                .clearColor = meno::Color::fromRgb(0x1E2430)}) {}

private:
    void onStart() override {
        auto& scene = scenes().createScene();
        auto& player = scene.createGameObject();
        player.transform().pos = {480.f, 270.f};

        if (!scenes().changeScene(scene.id())) {
            throw std::runtime_error("Failed to activate scene");
        }
    }
};

int main() {
    MyGame game;
    game.run();
}
```

## 고정된 실행 순서

`Game`은 다음 `Application` 콜백을 `final`로 구현한다.

1. `processEvents`: `Window::pollEvents()`를 호출하고 창이 닫혔다면 `stop()`한다.
2. `update`: `SceneManager::update(fixedDeltaTime)`을 호출한다. Scene update handler가
   끝나면 예약된 객체 생성·삭제와 장면 전환이 기존 규칙에 따라 반영된다.
3. `render`: `Renderer::beginFrame()` → 활성 Scene 그리기 → `endFrame()` 순서로 실행한다.
4. `onStop`: Scene을 모두 정리한다.

게임별로 이 순서를 빠뜨리지 않도록 세 함수와 `onStop`은 재정의할 수 없다.
보간은 아직 Scene/GameObject의 이전 상태를 저장하는 공통 계약이 없으므로
`interpolationAlpha`를 장면 렌더링에 적용하지 않는다. 직접 보간 렌더링이 필요한
특수한 프로그램은 기존 `Application`을 사용하면 된다.

## 접근자와 수명

- `window()`: 입력을 포함한 게임 창
- `renderer()`: 창에 연결된 2D 렌더러
- `resources()`: 경로별 리소스 캐시
- `scenes()`: `SceneManager::instance()`에 대한 참조

`SceneManager`는 프로세스 전역 싱글턴이므로 동시에 살아 있는 `Game`은 하나만
허용한다. 첫 번째 `Game`이 소멸하기 전에 다른 `Game`을 생성하면
`std::logic_error`를 던져 서로의 Scene을 정리하는 문제를 방지한다.

종료 시 Scene을 가장 먼저 비운 뒤 `ResourceManager`, `Renderer`, `Window` 순서로
정리한다. 정상 종료의 Scene exit handler가 던진 예외는 `run()` 호출자에게 전달한다.
`onStart`, update 또는 render가 예외를 던지면 `Game::run()`이 파생 클래스의 멤버가
파괴되기 전에 Scene을 정리하고 원래 예외를 다시 던진다. `Application&` 또는
`Application*`로 실행해도 가상 `run()`을 통해 같은 정리 경로를 사용한다.

마지막 방어선인 `Game` 소멸자가 실행될 때는 파생 클래스 멤버가 이미 파괴된
상태다. 따라서 이 경로에서는 `onExit`을 호출하지 않고 남은 Scene만 폐기한다.
정상 종료와 `run()` 내부 예외에서는 파생 객체가 살아 있을 때 `onExit`을 호출한다.
`run()`의 재진입 검사는 루프가 실행 중인 구간뿐 아니라 `onStop`과 Scene의
`onExit`이 끝날 때까지 유지된다. 따라서 종료 콜백에서도 중첩 `run()`은
`std::logic_error`로 거부된다.

## 예제 변경량

중복된 창·렌더러·SceneManager 멤버와 루프 콜백을 제거한 결과다.

| 예제 | 변경 전 | Game 적용 후 | 감소 |
|---|---:|---:|---:|
| `jumping_box` | 139줄 | 125줄 | 14줄 |
| `scene_transition` | 107줄 | 83줄 | 24줄 |
| `gameobject_demo` | 67줄 | 63줄 | 4줄 |

`scene_transition`의 진행 막대처럼 Scene 밖에서 직접 그리던 표현은 제거했다.
통합 경로에서는 화면에 보이는 요소도 Scene/GameObject로 구성하는 것이 원칙이다.
