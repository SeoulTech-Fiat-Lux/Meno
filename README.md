# meno

SFML 기반 2D 게임 개발 라이브러리. 서울과학기술대학교 캡스톤디자인(2) 프로젝트.

게임 루프·객체 관리·충돌 판정·리소스 관리처럼 게임마다 반복 구현되는 기능을
상위 라이브러리로 제공한다. **사용자는 SFML을 직접 다루지 않는다.**

```cpp
#include <meno/Meno.hpp>

class MyGame final : public meno::Game {
private:
    void onStart() override {
        auto& scene = scenes().createScene();
        // scene에 GameObject와 handler를 구성한다.
        (void)scenes().changeScene(scene.id());
    }
};

int main() {
    MyGame game;
    game.run();
}
```

## 빌드

요구 사항: **CMake 3.28+**, **C++20 컴파일러** (MSVC 2022 / GCC 13+ / Clang 16+)

SFML은 CMake가 자동으로 받아온다. Windows는 별도 설치가 필요 없다.
Linux는 SFML이 쓰는 시스템 개발 패키지를 먼저 설치한다(Ubuntu 기준, CI와 같은 목록).

```bash
sudo apt-get install -y \
  libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
  libfreetype-dev libgl1-mesa-dev libegl1-mesa-dev \
  libflac-dev libvorbis-dev libogg-dev \
  libharfbuzz-dev
```

```bash
cmake -S . -B build
cmake --build build
```

> 첫 빌드는 10분쯤 걸린다. SFML과 그 의존성(freetype·harfbuzz·SheenBidi)을
> 함께 빌드하기 때문이다. 두 번째부터는 빠르다.

예제 실행:

```bash
./build/examples/meno_example_render_basics
./build/examples/meno_example_input_demo
```

자동 이동·점프 예제: `./build/examples/meno_example_jumping_box`
— 바닥과 상자 GameObject를 만들고, 상자가 좌우로 왕복하며 점프한다.
[예제 안내](examples/jumping_box/README.md)에서 구성과 조절 가능한 값을 확인할 수 있다.

씬 전환 예제: `./build/examples/meno_example_scene_transition`
— 파란 씬과 주황 씬이 3초마다 전환된다.
씬 전환 콜백의 흐름은 [예제 코드](examples/scene_transition/main.cpp)에서 확인할 수 있다.

테스트:

```bash
ctest --test-dir build --output-on-failure
```

## 내 프로젝트에 넣기

CMake의 FetchContent로 가져온다. SFML도 함께 받아 링크하므로 따로 연결하지 않는다.

```cmake
cmake_minimum_required(VERSION 3.28)
project(my_game LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    meno
    GIT_REPOSITORY https://github.com/SeoulTech-Fiat-Lux/Meno.git
    GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(meno)

add_executable(my_game main.cpp)
target_link_libraries(my_game PRIVATE meno::meno)
```

- 저장소를 프로젝트 안에 받아 두었다면 FetchContent 대신 `add_subdirectory(<meno 경로>)`를 쓴다.
- 이렇게 가져가면 meno의 테스트와 예제는 기본으로 꺼진다.
- meno는 사용자 프로젝트의 `CMAKE_BUILD_TYPE`과 `BUILD_SHARED_LIBS`를 바꾸지 않는다.

## 고정 timestep 게임 루프

일반적인 게임은 `Game`을 상속한다. `Game`이 창, 렌더러, 리소스 캐시와
SceneManager를 소유·연결하고 이벤트 처리, fixed update, 장면 렌더링과 종료 정리를
수행한다. 자세한 실행 순서와 수명 규칙은 [Game 통합 실행 경로](docs/game-api.md)에 있다.

`Application`을 상속하면 사용자가 `while` 루프나 시간 누적을 직접 작성할
필요가 없다. `update()`는 고정 간격으로 0회 이상 호출되고 `render()`는 프레임당
한 번 호출된다. 렌더 인자는 직전/현재 게임 상태를 보간할 때 쓰는 0~1 사이의 비율이다.

```cpp
class Game final : public meno::Application {
public:
    using Application::Application;

private:
    meno::Window window{{960u, 540u}, "My game"};

    void processEvents() override {
        window.pollEvents();
        if (!window.isOpen()) stop();
    }

    void update(double fixedDeltaTime) override {
        // 게임 상태 갱신
    }

    void render(double interpolationAlpha) override {
        // 보간해 렌더
    }
};

int main() {
    Game game({.framerateLimit = 60});
    game.run();
}
```

`Time::deltaTime()`, `Time::fixedDeltaTime()`, `Time::elapsedTime()`으로 fixed update 기준의
시뮬레이션 시간을 읽을 수 있다. 프레임 기준의 가변 시간과 실제 누적 시간은
`Time::frameDeltaTime()`, `Time::realElapsedTime()`으로 읽는다. `frameDeltaTime()`은
`fixedTimeStep * maxUpdatesPerFrame`으로 제한되지만 `realElapsedTime()`은 제한되지 않는다.
프레임 제한이 필요하면
`ApplicationConfig::framerateLimit`을 사용하며 0이면 제한하지 않는다.
`run(Clock&)`은 사용자 정의 또는 테스트용 `Clock`을 주입할 때 사용하며, 이 경우에도
설정된 프레임 제한이 적용된다.

## 씬 상태

`Scene::setState<T>(std::unique_ptr<T>)`는 씬 고유 상태의 소유권을 Scene으로 이전한다.
`getState<T>()`로 같은 객체를 조회할 수 있으므로, 등록 전에 포인터를 따로 꺼낼 필요가 없다.
상태를 먼저 등록한 뒤 해당 상태를 사용하는 핸들러를 설정한다.

```cpp
struct GameState {
    double elapsed = 0.0;
    int score = 0;
};

meno::SceneManager scenes;
auto& scene = scenes.createScene();
scene.setState(std::make_unique<GameState>());

scene.setUpdateHandler([&scene](double dt) {
    auto* state = scene.getState<GameState>();
    state->elapsed += dt;
});

const meno::Scene& view = scene;
const GameState* state = view.getState<GameState>();
```

- `getState<T>()`는 등록된 타입을 런타임에 검사하며, 타입이 다르면 `nullptr`을 반환한다.
- 상태가 없으면 `nullptr`을 반환한다. `const Scene`에서는 `const T*`를 반환한다.
- 반환 포인터는 빌린 포인터다. 호출자가 `delete`하거나 다른 소유 포인터로 감싸면 안 된다.
  상태는 Scene이 소멸할 때 함께 파괴되므로, Scene 삭제 또는 `SceneManager::shutdown()`으로
  Scene이 정리된 뒤에는 사용할 수 없다. 씬 전환만으로 상태가 파괴되지는 않는다.
- 유효한 상태가 이미 있으면 `setState`는 `std::logic_error`를 던진다. 기존 상태는 유지되고,
  새로 전달한 객체는 정리된다. 상태를 교체하거나 꺼내 소유권을 돌려받는 API는 없다.
- 상태가 없을 때 빈 `unique_ptr`을 넘기면 빈 상태로 남으며, 이후 다시 등록할 수 있다.

## 그리기

`Renderer`는 부른 순서대로 그린다. 나중에 부른 것이 위에 올라간다.
좌표는 픽셀이고 원점은 창 좌상단, y는 아래로 증가한다. 각도는 전부 도(degree) 단위이며 시계 방향이 +다.

```cpp
renderer.beginFrame(meno::Color::fromRgb(0x14181F)); // 배경색으로 지운다

renderer.drawRect({{40.f, 40.f}, {120.f, 16.f}}, {.fill = meno::colors::White});
renderer.drawCircle(ball.position, 8.f, {.fill = meno::Color::fromRgb(0xE5533D)});
renderer.drawLine(from, to, {.color = meno::Color::fromRgb(0x4FC3F7), .thickness = 2.f}); // 두께 2px

// 다각형. 점은 position 기준 로컬 좌표이고, rotation은 position을 축으로 돈다.
renderer.drawPolygon(ship.position, {{14.f, 0.f}, {-10.f, -8.f}, {-10.f, 8.f}},
                     {.fill = meno::colors::White, .rotation = ship.angle});

// 외곽선만 그리려면 fill을 투명으로 둔다. std::vector나 std::array도 그대로 넘긴다.
renderer.drawPolygon(rock.position, rock.points,
                     {.fill = meno::colors::Transparent, .outline = meno::colors::White, .outlineThickness = 2.f});

// 부채꼴. 0도가 오른쪽이다. 오른쪽을 보고 입을 60도 벌린 팩맨:
renderer.drawSector(pac.position, 12.f, 30.f, 300.f, {.fill = meno::colors::Yellow});

renderer.endFrame();
```

색은 이렇게 만든다.

| 만드는 법 | 예 |
|---|---|
| 6자리 `#RRGGBB` | `meno::Color::fromRgb(0x4FC3F7)` |
| 8자리 `#RRGGBBAA` | `meno::Color::fromRgba(0x4FC3F780)` |
| 반투명 | `meno::colors::White.withAlpha(128)` (0 투명 ~ 255 불투명) |
| 이름 있는 색 | `meno::colors::White`, `Black`, `Red`, `Yellow`, `Transparent` 등 |

- **`fromRgb`에는 6자리만 넣는다.** 8자리를 넣으면 상수 식에서는 컴파일 에러, 그 밖에서는 마젠타(`#FF00FF`)가 나온다. 게임은 멈추지 않는다.
- **반투명은 `withAlpha`로 만든다.** `fromRgb(0x00000080)`처럼 앞자리가 `00`인 8자리는 오류로 잡히지 않고 불투명한 남색이 된다.
- **텍스처에 색을 곱하려면 `tint`를 쓴다.** 흰색이면 원본 그대로다.

```cpp
renderer.draw(playerTexture, {.position = player.position, .tint = meno::Color::fromRgb(0xFF6060)}); // 붉게
renderer.draw(playerTexture, {.position = player.position, .tint = meno::colors::White.withAlpha(128)}); // 반투명
```

카메라, 텍스트 정렬, 다각형 채우기 조건 같은 자세한 규칙은 [docs/render-api.md](docs/render-api.md)에 있다.

## 오디오

짧은 효과음은 파일 전체를 메모리에 올리는 `SoundBuffer`와 이를 재생하는 `Sound`,
긴 배경음악은 파일에서 스트리밍하는 `Music`을 사용한다. 이 타입들은 복사할 수 없고 이동할 수 있으며, 볼륨은
`0.0f`(무음)부터 `1.0f`(최대)까지 지정한다.

```cpp
auto click = meno::Sound::loadFromFile("assets/click.wav");
if (click) {
    click->setVolume(0.7f);
    click->play();
}

auto bgm = meno::Music::openFromFile("assets/bgm.ogg");
if (bgm) {
    bgm->setVolume(0.4f);
    bgm->setLooping(true); // 끝까지 재생하면 처음부터 반복
    bgm->play();
}
```

`Music::setLooping(true)`으로 배경음악을 반복 재생할 수 있으며 기본값은 `false`다.
현재 설정은 `Music::isLooping()`으로 확인한다. 효과음용 `Sound`에는 반복 재생 API를
제공하지 않는다.

`Sound`와 `Music` 객체가 소멸하면 재생도 즉시 중단된다. 따라서 다음처럼 임시
객체에서 바로 `play()`를 호출하지 말고, 재생이 끝날 때까지 지역 변수나 게임 객체의
멤버로 보관해야 한다.

`Sound::loadFromFile()`은 단독 사용을 위한 편의 함수라 호출할 때마다 파일을 로드한다.
같은 효과음을 여러 재생 객체가 공유해야 하면 아래의 `ResourceManager`에서
`SoundBuffer`를 불러와 각각의 `Sound`에 전달한다.

```cpp
// 잘못된 예: 문장이 끝날 때 임시 객체가 소멸하여 소리가 재생되지 않는다.
meno::Sound::loadFromFile("assets/click.wav")->play();

// 올바른 예: 재생 중에도 click이 유지된다.
auto click = meno::Sound::loadFromFile("assets/click.wav");
if (click) {
    click->play();
}
```

`Music`은 재생 중 파일을 계속 읽으므로 해당 객체가 살아 있는 동안 원본 파일을
이동하거나 삭제하면 안 된다. 로딩 또는 열기에 실패하면 두 함수 모두
`std::nullopt`를 반환한다.

## 입력

`Window`가 키보드·마우스 입력 상태(`Input`)를 하나 갖고 있고, `pollEvents()`가 그것을 갱신한다.

```cpp
void processEvents() override {
    window.pollEvents();
    if (!window.isOpen()) stop();
}

void update(double fixedDeltaTime) override {
    const meno::Input& input = window.input();
    if (input.isDown(meno::Key::D)) { /* 누르고 있는 동안 계속 */ }
    if (input.wasPressed(meno::Key::Space)) { /* 한 번 누르면 한 번만 */ }
    if (input.wasPressed(meno::MouseButton::Left)) {
        const meno::Vec2i clicked = input.mousePosition(); // 창 기준 픽셀
    }
}
```

`wasPressed`·`wasReleased`·`mouseWheelDelta()`·`text()`는 "이번 틱"에 일어난 일만 돌려준다.
`update()` 안에서는 fixed update 한 번이 한 틱이다. 프레임에 update가 0번 돌든 여러 번 돌든,
한 번 누른 키는 그 뒤 처음 도는 update 한 번에서만 true다. `render()`나 직접 짠 루프에서는
`pollEvents()` 한 번이 한 틱이다.

키 여러 개를 이름 하나로 묶으려면 액션 매핑을 쓴다.

```cpp
input.bind("jump", meno::Key::Space);
input.bind("jump", meno::Key::W);
input.bind("jump", meno::MouseButton::Left);

if (input.wasActionPressed("jump")) { /* Space를 누른 채 W를 더 눌러도 한 번만 */ }
```

포커스를 잃었을 때, 텍스트 입력, 액션 판정 규칙은 [docs/input-api.md](docs/input-api.md)에 있다.

## 리소스 캐시

`ResourceManager`는 같은 경로의 `Texture`, `Font`, `SoundBuffer`를 한 번만 로드하고 공유한다.
성공하면 `std::shared_ptr<const T>`, 실패하면 빈 `shared_ptr`을 반환한다. 반환된
리소스는 매니저가 소멸해도 이를 참조하는 `shared_ptr`이 남아 있는 동안 유지된다.

```cpp
meno::ResourceManager resources;

auto texture = resources.loadTexture("assets/player.png", {.smooth = true});
if (texture) {
    gameObject.sprite().setTexture(texture);
}

auto font = resources.loadFont("assets/main.ttf");
if (font) {
    renderer.drawText(*font, "Hello", {10.0f, 10.0f});
}

auto shotBuffer = resources.loadSoundBuffer("assets/shot.wav");
// 재생이 끝날 때까지 아래 객체를 유지해야 한다.
meno::Sound first{shotBuffer};
meno::Sound second{shotBuffer};
first.play();
second.play(); // 같은 샘플 데이터를 공유하며 독립적으로 재생
```

동일한 정규화 경로와 옵션을 다시 요청하면 같은 객체를 가리키는 `shared_ptr`을 반환한다.
텍스처의 `smooth` 옵션이 다르면 별도의 텍스처로 캐시한다.
로드 실패는 캐시하지 않으므로 파일을 수정한 뒤 다시 요청할 수 있다.

## 디렉터리 구조

```
include/meno/     공개 헤더 — 여기에 SFML은 등장하지 않는다
  Meno.hpp          통합 헤더
  math/             Vec2, Rect, Color, Angle, Camera2D
  core/             Application, Clock, Time
  input/            Input, Key, MouseButton, Event
  window/           Window
  graphics/         Renderer, Texture, Font, DrawParams
  audio/            SoundBuffer, Sound, Music
  resources/        ResourceManager
  scene/            Scene, SceneManager, GameObject, Component와 Transform·Sprite·Collider·AudioSource
  app/              Game
src/meno/         구현 — 사용자에게 배포되지 않는다. SFML은 여기서만 include한다
  sfml_backend/     SFML과 meno 타입 사이의 변환, 내부 접근 통로
examples/         데모 게임
tests/            테스트 (consumer/는 하위 프로젝트 빌드 검사)
tools/            check_headers.py — 공개 헤더 SFML 유출 검사
docs/             설계 문서
```

각 모듈은 위 목록에서 자기보다 위에 있는 모듈만 참조한다. 모듈 사이에 순환 의존이 없다.

`include/`와 `src/`가 나뉜 게 핵심이다. CMake에서 `include/`는 `PUBLIC`,
`src/`는 `PRIVATE`으로 지정해 **SFML이 사용자 빌드로 새지 않게** 한다.
자세한 근거는 [docs/render-api.md](docs/render-api.md)에 있다.

검사는 자동화돼 있다:

```bash
python tools/check_headers.py
```

## 문서

| 문서 | 내용 |
|---|---|
| [docs/render-api.md](docs/render-api.md) | 2D 렌더 API 설계 근거, 색·도형 규칙, 담당자 간 계약, SFML 3 변경점 |
| [docs/game-api.md](docs/game-api.md) | Game 통합 루프, 접근자, 종료 순서와 예제 변경량 |
| [docs/input-api.md](docs/input-api.md) | 입력 API 구조, "이번 틱" 판정 규칙, 액션 매핑 규칙 |
| [docs/collision-api.md](docs/collision-api.md) | 콜라이더 등록, 객체 간 충돌 검사, 점 포함 검사와 사용 예제 |

## 개발

작업은 이슈 단위로 브랜치를 따서 PR로 `main`에 병합한다.

- 브랜치: `<유형>/#<이슈 번호>-<설명>` (예: `Bugfix/#26-Project-Compatibility`)
- 커밋: `<유형>: <설명> #<이슈 번호>` (예: `fix: Ignore non-positive radius #35`)

`math/`는 렌더러와 충돌 판정이 함께 쓰는 공용 타입이라, 변경 시 양쪽 리뷰를 거친다.

코드 스타일은 `.clang-format`을 따른다. 버전마다 결과가 조금씩 달라지므로 **clang-format 21.1.7**을 쓴다.
저장 시 자동 포맷을 켜두면 된다.

```bash
pip install clang-format==21.1.7
```

저장소 전체를 포맷한 커밋은 `.git-blame-ignore-revs`에 모아 두었다.
클론한 뒤 한 번 설정하면 `git blame`이 이 커밋들을 건너뛴다.

```bash
git config blame.ignoreRevsFile .git-blame-ignore-revs
```

## 라이선스

[zlib](LICENSE) — 의존하는 SFML과 동일하다.
