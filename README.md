<!-- 로고는 GitHub 테마 설정을 따라 하나만 보인다(#gh-light-mode-only, #gh-dark-mode-only).
     <picture>는 OS 설정만 따르므로, GitHub만 다크일 때 검은 글자 로고가 나와 글자가
     보이지 않는다. -->
<p align="center">
  <a href="https://seoultech-fiat-lux.github.io/Meno/#gh-light-mode-only">
    <img src="docs/theme/logo/meno-logo-on-light.svg" alt="meno" width="360">
  </a>
  <a href="https://seoultech-fiat-lux.github.io/Meno/#gh-dark-mode-only">
    <img src="docs/theme/logo/meno-logo-on-dark.svg" alt="meno" width="360">
  </a>
</p>

<p align="center">
  SFML 기반 C++20 2D 게임 개발 라이브러리
</p>

<p align="center">
  <a href="https://seoultech-fiat-lux.github.io/Meno/guide.html">설명서</a>
  ·
  <a href="https://seoultech-fiat-lux.github.io/Meno/">API 레퍼런스</a>
  ·
  <a href="examples">예제</a>
  ·
  <a href="#시작하기">시작하기</a>
</p>

서울과학기술대학교 캡스톤디자인(2) 프로젝트인 meno는 SFML 3 위에서 2D 게임을 만드는 C++20 라이브러리입니다.

## 짧은 예제

방향키로 원을 움직입니다. 그림이나 글꼴 파일 없이 그대로 실행됩니다.

```cpp
#include <meno/Meno.hpp>

int main() {
    meno::Game game{{.windowSize = {640, 480}, .title = "Hello meno"}};
    meno::Vec2f position{320.f, 240.f};

    meno::Scene& play = game.scenes().createScene();
    play.setUpdateHandler([&](double dt) {
        const meno::Input& input = game.window().input();
        const float step = 240.f * static_cast<float>(dt); // 1초에 240픽셀
        if (input.isDown(meno::Key::Left)) {
            position.x -= step;
        }
        if (input.isDown(meno::Key::Right)) {
            position.x += step;
        }
        if (input.isDown(meno::Key::Up)) {
            position.y -= step;
        }
        if (input.isDown(meno::Key::Down)) {
            position.y += step;
        }
    });
    play.setDrawHandler([&](meno::Renderer& renderer) {
        renderer.drawCircle(position, 24.f, {.fill = meno::Color::fromRgb(0x00B1ED)});
    });
    (void)game.scenes().changeScene(play.id());
    game.run(); // 창을 닫으면 돌아옵니다.
}
```

`Game`이 창, 렌더러, 리소스, 씬을 묶어 게임 루프를 돌립니다. update 핸들러는 고정 시간 간격으로,
draw 핸들러는 프레임마다 불립니다.

## 시작하기

CMake 3.28 이상과 C++20 컴파일러(MSVC 2022, GCC 13 이상, Clang 16 이상)가 필요합니다.
SFML 3은 CMake가 함께 받아 빌드하므로 따로 설치하지 않습니다.

```cmake
cmake_minimum_required(VERSION 3.28)
project(my_game LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    meno
    GIT_REPOSITORY https://github.com/SeoulTech-Fiat-Lux/Meno.git
    GIT_TAG        v0.2.0
)
FetchContent_MakeAvailable(meno)

add_executable(my_game main.cpp)
target_link_libraries(my_game PRIVATE meno::meno)
```

- 첫 빌드는 SFML과 그 의존성까지 빌드하므로 조금 걸립니다. 두 번째부터는 빠릅니다.
- 저장소를 프로젝트 안에 받아 두었다면 FetchContent 대신 `add_subdirectory(<meno 경로>)`를 씁니다.
- 이렇게 가져가면 meno의 테스트와 예제는 빌드하지 않으며, 사용자 프로젝트의 `CMAKE_BUILD_TYPE`과
  `BUILD_SHARED_LIBS`도 바꾸지 않습니다.

<details>
<summary>Linux에서는 SFML이 쓰는 시스템 패키지를 먼저 설치합니다 (Ubuntu)</summary>

CI와 같은 목록입니다. Windows는 따로 설치할 것이 없습니다.

```bash
sudo apt-get install -y \
  libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
  libfreetype-dev libgl1-mesa-dev libegl1-mesa-dev \
  libflac-dev libvorbis-dev libogg-dev \
  libharfbuzz-dev
```

</details>

예제를 바로 실행하려면, 이 저장소를 빌드하면 됩니다.

```bash
git clone https://github.com/SeoulTech-Fiat-Lux/Meno.git
cd Meno
cmake -S . -B build
cmake --build build
./build/examples/meno_example_input_demo
```

Visual Studio 생성기(Windows의 기본값)를 쓰면 실행 파일은 `build/examples/Debug/`에 생깁니다.
다른 예제는 [examples](examples)에 있습니다.

## 문서

- [사용자 설명서](https://seoultech-fiat-lux.github.io/Meno/guide.html): 설치와 첫 게임부터
  GameObject, 충돌 판정, 씬 전환, 오디오, 카메라까지 게임을 만드는 순서에 따라 설명합니다. 원문은
  [docs/guide](docs/guide)에 있습니다.
- [API 레퍼런스](https://seoultech-fiat-lux.github.io/Meno/): 클래스·함수마다 동작, 주의할 점, 예제를
  적었습니다. 게임, 씬, 그래픽, 입력 같은 모듈별로 찾아볼 수 있습니다.
- [예제](examples): 실행해 볼 수 있는 작은 데모 게임입니다.

## 한계

0.x 버전이므로 API가 바뀔 수 있습니다. 지금은 다음과 같은 한계가 있습니다.

- 3D, 네트워크, 물리 시뮬레이션, 타일맵, UI 위젯은 없습니다.
- 충돌은 겹치는지만 판정하며, 사각형과 원형 콜라이더만 제공합니다.
- 한 번에 하나의 `Game`(`Application`)만 실행할 수 있습니다.
- `ResourceManager`는 한 번 읽은 리소스를 매니저가 사라질 때까지 캐시에 둡니다.
- Windows(MSVC)와 Ubuntu(GCC, Clang)에서 빌드하고 테스트합니다. macOS에서는 확인하지
  않았습니다.

## 기여하기

빌드와 테스트, 저장소 구조, 브랜치·커밋 규칙, 코드 스타일은 [CONTRIBUTING.md](CONTRIBUTING.md)에
있습니다.

## 라이선스

[zlib](LICENSE).