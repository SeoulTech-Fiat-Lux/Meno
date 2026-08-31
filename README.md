# meno

SFML 기반 2D 게임 개발 라이브러리. 서울과학기술대학교 캡스톤디자인(2) 프로젝트.

게임 루프·객체 관리·충돌 판정·리소스 관리처럼 게임마다 반복 구현되는 기능을
상위 라이브러리로 제공한다. **사용자는 SFML을 직접 다루지 않는다.**

```cpp
#include <meno/Meno.hpp>

int main() {
    meno::Window window{{960u, 540u}, "내 게임"};
    meno::Renderer renderer{window};

    // 일반 게임은 meno::Application을 상속해 update/render만 구현한다.
    // Application이 고정 timestep 루프와 시간 누적을 담당한다.
    while (window.isOpen()) { // 렌더 API만 보여 주는 최소 예제
        window.pollEvents();
        renderer.beginFrame(meno::Color::fromHex(0x1E2430FF));
        renderer.drawCircle({480.f, 270.f}, 60.f, {.fill = meno::colors::Cyan});
        renderer.endFrame();
    }
}
```

## 빌드

요구 사항: **CMake 3.28+**, **C++20 컴파일러** (MSVC 2022 / GCC 13+ / Clang 16+)

SFML은 CMake가 자동으로 받아온다. 별도 설치가 필요 없다.

```bash
cmake -S . -B build
cmake --build build
```

> 첫 빌드는 10분쯤 걸린다. SFML과 그 의존성(freetype·harfbuzz·SheenBidi)을
> 함께 빌드하기 때문이다. 두 번째부터는 빠르다.

예제 실행:

```bash
./build/examples/meno_example_render_basics
```

테스트:

```bash
ctest --test-dir build --output-on-failure
```

## 고정 timestep 게임 루프

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

## 디렉터리 구조

```
include/meno/     공개 헤더 — 여기에 SFML은 등장하지 않는다
  Meno.hpp          통합 헤더
  core/             Window, Application, Clock, Time
  math/             Vec2, Rect, Color, Camera2D
  graphics/         Renderer, Texture, Font, DrawParams
src/meno/         구현 — 사용자에게 배포되지 않는다
  sfml_backend/     SFML을 include하는 유일한 곳
examples/         데모 게임
tests/            테스트
tools/            check_headers.py — 공개 헤더 SFML 유출 검사
docs/             설계 문서
```

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
| [docs/render-api.md](docs/render-api.md) | 2D 렌더 API 설계 근거, 담당자 간 계약, SFML 3 변경점 |
| [docs/cpp-guide.md](docs/cpp-guide.md) | 이 코드로 배우는 현업 C++ — pimpl·RAII·소유권 |

## 개발

브랜치는 기능 단위로 파고 PR로 병합한다.

| 브랜치 | 담당 영역 |
|---|---|
| `2D_Renderer` | `graphics/`, `math/`, `sfml_backend/` |
| `GameObject-Demo` | `scene/`, `collision/` |

`math/`는 렌더러와 충돌 판정이 함께 쓰는 공용 타입이라, 변경 시 양쪽 리뷰를 거친다.

코드 스타일은 `.clang-format`이 강제한다. 저장 시 자동 포맷을 켜두면 된다.

## 라이선스

[zlib](LICENSE) — 의존하는 SFML과 동일하다.
