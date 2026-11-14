# 1장. 설치와 첫 빌드

이 장에서는 사용자의 CMake 프로젝트에 meno를 추가하고 빈 창 하나를 띄웁니다.

## 필요한 것

- CMake 3.28 이상
- C++20 컴파일러: MSVC 2022, GCC 13 이상, Clang 16 이상

SFML 3은 CMake가 meno와 함께 내려받아 빌드하므로 따로 설치할 필요가 없습니다. meno의 공개 헤더에는
SFML이 드러나지 않아 게임 코드에서 SFML을 직접 다룰 일도 없습니다.

Linux(Ubuntu)에서는 SFML이 쓰는 시스템 패키지를 먼저 설치합니다. 아래 목록은 CI에서 설치하는 것과
같습니다.

```bash
sudo apt-get install -y \
  libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
  libfreetype-dev libgl1-mesa-dev libegl1-mesa-dev \
  libflac-dev libvorbis-dev libogg-dev \
  libharfbuzz-dev
```

## 프로젝트 만들기

폴더를 다음과 같이 구성합니다.

```
my_game/
├── CMakeLists.txt
└── main.cpp
```

`CMakeLists.txt`에서는 FetchContent로 meno를 내려받습니다.

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

`meno::meno`에 링크하면 C++20 설정과 헤더 경로가 함께 적용되며, SFML을 따로 링크할 필요도 없습니다.
MSVC에서는 한글 문자열이 깨지지 않도록 `/utf-8` 옵션도 켜집니다.

`main.cpp`에서는 창을 띄우고, 창이 닫힐 때까지 기다립니다.

```cpp
#include <meno/Meno.hpp>

int main() {
    meno::Game game{{.windowSize = {640, 480}, .title = "My game"}};
    game.run(); // 창을 닫으면 돌아옵니다.
}
```

`<meno/Meno.hpp>`는 meno의 공개 헤더를 모두 include합니다. 컴파일 시간을 줄이려면
`<meno/app/Game.hpp>`처럼 필요한 헤더만 include해도 됩니다.

## 빌드와 실행

```bash
cmake -S . -B build
cmake --build build
./build/my_game
```

검은 창이 뜨고, 닫기 버튼을 눌렀을 때 프로그램이 끝나면 성공입니다.

- 첫 빌드에서는 SFML과 그 의존 라이브러리까지 컴파일하므로 10분 정도 걸립니다. 두 번째 빌드부터는
  빠릅니다.
- Visual Studio 생성기(Windows의 기본값)를 쓰면 실행 파일은 `build/Debug/my_game.exe`에
  만들어집니다.

meno 저장소를 프로젝트 안에 내려받아 두었다면 FetchContent 대신 `add_subdirectory(<meno 경로>)`를
씁니다. 어느 방식이든 meno는 자신의 테스트와 예제를 빌드하지 않으며, 사용자 프로젝트의
`CMAKE_BUILD_TYPE`과 `BUILD_SHARED_LIBS`도 바꾸지 않습니다.

## 에셋 파일 두기

그림, 폰트, 효과음 파일은 보통 프로젝트의 `assets/` 폴더에 두고 `"assets/player.png"`처럼 상대
경로로 읽습니다.

```
my_game/
├── CMakeLists.txt
├── main.cpp
└── assets/
    ├── player.png
    └── fonts/
        └── game.ttf
```

상대 경로는 실행 파일의 위치가 아니라 **프로그램을 실행한 현재 작업 디렉터리**를 기준으로
해석됩니다. 위 예처럼 프로젝트 폴더에서 `./build/my_game`을 실행하면 `my_game/assets/`에서 파일을
찾습니다. IDE에서 실행할 때는 작업 디렉터리가 빌드 폴더일 수 있으므로, 실행 설정에서 작업 디렉터리를
프로젝트 폴더로 지정합니다.

meno는 파일을 읽지 못해도 예외를 던지지 않고 빈 값을 돌려줍니다([3장](03-drawing.md)). 그림이나
글자가 보이지 않으면 경로부터 확인합니다.

## 주의할 점

- 파일 이름의 대소문자는 실제 파일과 똑같이 씁니다. 대소문자가 틀려도 Windows에서는 파일이 열리지만,
  Linux에서는 열리지 않습니다.
- 한글 문자열이 들어간 소스 파일은 UTF-8로 저장합니다. 화면에 한글을 그리려면 한글 글리프가 있는
  폰트도 필요합니다([3장](03-drawing.md)).
- `GIT_TAG`는 `v0.2.0` 같은 태그로 고정합니다. `main` 같은 브랜치 이름을 쓰면 내려받을 때마다 내용이
  달라질 수 있습니다.

## 관련 예제와 API

- API: [Game](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_game.html),
  [GameConfig](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_game_config.html)
- 예제 모음: [examples](https://github.com/SeoulTech-Fiat-Lux/Meno/tree/v0.2.0/examples). meno
  저장소를 빌드하면 예제도 함께 빌드되며, 모두 에셋 파일 없이 실행됩니다.

다음 장: [2장. 첫 게임: Game과 Scene](02-first-game.md)
