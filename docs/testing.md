# 부록. 테스트와 CI

이 부록은 meno가 기능을 어떻게 검증하는지 정리합니다. 평가자와 기여자를 위한 문서이며, 게임을 만들
때는 읽지 않아도 됩니다.

## 테스트 실행

meno를 최상위 프로젝트로 빌드하면 테스트도 함께 빌드됩니다(`MENO_BUILD_TESTS`). 저장소 루트에서 다음
명령으로 빌드하고 테스트를 실행합니다.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

창을 여는 테스트 5개는 기본으로 건너뜁니다. 디스플레이가 없는 곳에서 창을 만들면 SFML이 프로세스를
중단시키기 때문입니다. 이 테스트까지 실행하려면 환경 변수 `MENO_DISPLAY_TESTS`를 1로 지정합니다.

```bash
MENO_DISPLAY_TESTS=1 ctest --test-dir build --output-on-failure
```

PowerShell에서는 `$env:MENO_DISPLAY_TESTS = "1"`로 지정한 뒤 `ctest`를 실행합니다. 화면이 없는
Linux에서는 CI처럼 xvfb로 가상 디스플레이를 띄웁니다. `render_resize`와 `render_shapes`는 그린
픽셀의 색을 읽어 비교하므로 24비트 색 화면이 필요합니다. 다음과 같이 가상 화면의 형식을 지정합니다.

```bash
MENO_DISPLAY_TESTS=1 xvfb-run -a -s "-screen 0 1280x1024x24" \
  ctest --test-dir build --output-on-failure
```

- `render`는 글자를 그려 보므로 폰트 파일이 필요합니다. 먼저 환경 변수 `MENO_TEST_FONT`에 지정한
  경로를 쓰고, 지정하지 않았거나 그 파일이 없으면 DejaVu Sans(Linux)나 Arial(Windows, macOS)을
  찾습니다. 폰트를 찾지 못하면 건너뛰지 않고 실패합니다.
- Visual Studio 생성기처럼 구성이 여럿인 생성기로 빌드했다면 `-C Debug`처럼 구성을 지정합니다.
- 테스트 하나만 실행하려면 `-R`에 넘기는 이름을 `^`와 `$`로 감쌉니다. `-R`은 이름의 일부만 일치해도
  테스트를 고르므로, 예를 들어 `-R game`은 `gameobject`까지 실행합니다.

```bash
ctest --test-dir build -R "^scene_manager$" --output-on-failure
```

## 테스트의 구성

테스트는 프레임워크 없이 `tests/check.hpp`의 작은 검사기로 씁니다. 검사기는 검사마다 PASS나 FAIL을
출력하며, 실패가 하나라도 있으면 테스트가 종료 코드 1로 끝나 `ctest`가 실패로 집계합니다. CI는
Release로 빌드하므로, `NDEBUG`가 정의되면 사라지는 `assert()`는 쓰지 않습니다.

테스트는 검사할 코드가 무엇에 의존하는지에 따라 세 가지 방식으로 빌드합니다. 위쪽 방식일수록 SFML을
빌드하지 않고도 실행할 수 있고, 실패했을 때 원인을 좁히기 쉽습니다.

| 방식 | 빌드 | 예 |
|---|---|---|
| 헤더만 | 공개 헤더만 include합니다. | `math`, `angle`, `camera` |
| SFML 없는 구현 | 필요한 `src/` 파일만 함께 컴파일하고, 일부는 테스트 대역으로 바꿉니다. | `application`, `audio_component` |
| SFML 구현 | SFML을 포함한 meno와 링크합니다. | `render`, `scene` |

창이 필요한 테스트는 `MENO_DISPLAY_TESTS`가 비어 있거나 0이면 종료 코드 77로 끝나고, `ctest`는 이를
실패가 아닌 "건너뜀"으로 보고합니다.

`ctest`가 실행하지 않는 검사도 있습니다. 다음 검사는 configure나 빌드 단계에서 확인되거나, CI의 별도
단계에서 실행됩니다.

| 검사 | 확인하는 것 | 위치 |
|---|---|---|
| 헤더 독립 컴파일 | 공개 헤더마다(통합 헤더 `Meno.hpp` 포함) 그 헤더 하나만 include해도 컴파일되는지 | `tests/CMakeLists.txt`의 `meno_header_checks` |
| 설명서 코드 | 사용자 설명서, README, 사이트 첫 쪽의 C++ 코드 블록이 컴파일되는지 | `tests/guide/` |
| 테스트 등록 | `tests/` 아래 `test_*.cpp`가 모두 `meno_add_test`로 등록되었는지 | `tests/CMakeLists.txt` |
| SFML 유출 | 공개 헤더가 SFML 헤더를 include하거나 `sf::`를 쓰지 않는지 | `tools/check_headers.py` |
| 하위 프로젝트 빌드 | 사용자 프로젝트에 넣어도 빌드되고, 사용자의 CMake 캐시를 바꾸지 않는지 | `tests/consumer/` |

## 기능별 테스트

`ctest`에 등록된 테스트는 21개입니다. 빌드 열은 앞에서 본 세 방식(헤더, 구현, SFML)을 가리키며,
"창"이 붙은 테스트는 창을 엽니다.

| 모듈 | 테스트 | 빌드 | 검사하는 내용 |
|---|---|---|---|
| math | `math` | 헤더 | Vec2의 길이와 정규화(정수 벡터와 영벡터 포함), Rect의 `contains`(오른쪽·아래 경계 제외)와 `findIntersection`, Color 변환, 컴파일 시간 계산 |
| | `angle` | 헤더 | `toRadians()`의 변환, `wrapDegrees()`가 각도를 [0, 360)으로 접는 규칙(음수, 아주 큰 각도, NaN, 무한대 포함) |
| | `camera` | 헤더 | Camera2D의 기본값, 줌·회전·Letterbox·Stretch 계산, 분할 뷰포트 사이에 틈이나 겹침이 없는지, 잘못된 값에서 NaN이 나오지 않는지, 월드 좌표와 화면 좌표의 왕복 변환 |
| core | `application` | 구현 | 고정 시간 간격과 `maxUpdatesPerFrame`, 보간 계수의 범위, Time 값, 예외 뒤의 정리와 재실행, 한 번에 하나만 실행되는지, `onStart()`에 걸린 시간 제외, 잘못된 설정 거부, 프레임 제한 대기. 가짜 Clock을 주입해 실제로 기다리지 않습니다. |
| input | `input` | 구현 | 누름·유지·뗌, OS 반복 입력 무시, 한 프레임 안의 누름과 뗌, 휠 합산, UTF-8 글자 입력, 포커스를 잃을 때의 처리, 동작 묶기, update 한 번이 한 틱이라는 규칙 |
| | `input_convert` | SFML | SFML의 키, 버튼, 이벤트가 meno의 값으로 빠짐없이 변환되는지 |
| window | `window_input` | SFML·창 | `pollEvents()`와 `input()`이 같은 Input 객체를 유지하는지, Window를 복사·이동할 수 없는지 |
| graphics | `camera_parity` | SFML | 무작위 카메라 3000개에서 meno의 카메라 계산이 SFML의 뷰 변환과 일치하는지 |
| | `render` | SFML·창 | Texture와 Font의 이동 규칙, 이동된 객체로 그려도 안전한지, 텍스트 측정(외곽선, 여백, 빈 문자열), 카메라에 보이는 영역 |
| | `render_resize` | SFML·창 | 창 크기가 바뀐 뒤의 화면 좌표와 카메라 재적용, 레터박스 띠, 분할 뷰포트의 틈. 그린 픽셀을 읽어 확인합니다. |
| | `render_shapes` | SFML·창 | 사각형·원·선·다각형·부채꼴의 위치와 origin, 회전, 외곽선 방향, 부채꼴의 여러 각도, 잘못된 입력을 그리지 않는지. 그린 픽셀을 읽어 확인합니다. |
| audio | `audio` | SFML | Sound, SoundBuffer, Music의 이동 규칙, 빈 Sound와 AudioSource, 볼륨 규칙(0~1로 제한, NaN 무시), 이동한 뒤의 반복 재생 설정. 오디오 장치는 열지 않습니다. |
| resources | `resource_manager` | 구현 | 정리한 경로마다 한 번만 읽는지(유니코드 경로 포함), 옵션별 캐시, 실패를 캐시하지 않는지, 돌려받은 리소스가 매니저보다 오래 남는지 |
| scene | `scene` | SFML | GameObject ID(1부터 시작, 재사용하지 않음), 없는 ID와 두 번 지우기, 사용자가 Scene을 만들거나 복사할 수 없는지 |
| | `gameobject` | SFML | `addComponent()`와 `getComponent()`(Transform·Sprite 제외, 같은 타입은 교체), Sprite 그리기 옵션(오프셋의 회전·배율, 뒤집기), 컴포넌트를 복사할 때의 부모 |
| | `collider` | SFML | 사각형과 원 조합의 겹침 판정(경계만 닿으면 겹치지 않음), 배율과 회전 적용, 크기가 잘못되었거나 NaN일 때 판정하지 않는지, `contains()` |
| | `scene_draw_order` | 구현 | 레이어와 ID 순서의 그리기, update 핸들러 안에서 한 생성·삭제의 지연, draw 핸들러 규칙 |
| | `scene_manager` | 구현 | 전환 예약과 핸들러 호출 순서, 핸들러 안에서 거부되는 동작, enter·exit 핸들러의 예외 처리, `shutdown()` |
| | `scene_state` | 구현 | `setState()`와 `getState<T>()`, 두 번 등록할 때의 예외, 전환 뒤의 유지, Scene을 지울 때의 파괴 |
| | `audio_component` | 구현 | AudioSource의 읽기·교체·지우기, 볼륨 유지, 읽기에 실패해도 기존 소리를 유지하는지, GameObject와 함께 파괴되는지 |
| app | `game` | SFML·창 | 끝날 때 exit 핸들러와 컴포넌트가 창·리소스보다 먼저 정리되는지, `onStart()`에서 예외가 날 때의 정리, 중첩 실행 거부, Game마다 다른 SceneManager, 매 프레임 카메라 재적용 |

"구현" 테스트 가운데 `resource_manager`, `scene_draw_order`, `scene_manager`, `scene_state`,
`audio_component`는 SFML이 필요한 Texture, Sound, 그리기 등을 테스트 대역으로 바꿔 빌드합니다.

## CI

GitHub Actions의 CI 워크플로(`.github/workflows/ci.yml`)는 `main`에 푸시하거나 `main`을 대상으로
PR을 올릴 때 실행되며, 수동으로도 실행할 수 있습니다. 문서 워크플로(`.github/workflows/docs.yml`)는
공개 헤더, `docs/`, `Doxyfile`, README가 바뀔 때만 실행됩니다.

| 잡 | 환경 | 하는 일 |
|---|---|---|
| 공개 헤더 경계 검사 | Ubuntu | `tools/check_headers.py`로 공개 헤더의 SFML 유출을 검사합니다. 빌드보다 훨씬 빨라서, 문제가 있으면 먼저 실패합니다. |
| 하위 프로젝트 빌드 | Ubuntu / GCC | `tests/consumer/`를 빌드합니다. CMake 파일에 `CMAKE_SOURCE_DIR`가 쓰이지 않았는지도 확인합니다. |
| 빌드와 테스트 | Windows / MSVC, Ubuntu / GCC, Ubuntu / Clang | Release로 라이브러리, 예제, 테스트, 설명서 코드를 빌드하고 `ctest`를 실행합니다. Linux는 xvfb로 창 테스트까지 실행하고, Windows는 창 테스트를 건너뜁니다. |
| ASan·UBSan | Ubuntu / Clang | Debug 빌드에 AddressSanitizer와 UndefinedBehaviorSanitizer를 켜고 창 테스트까지 실행합니다. 누수 검사는 xvfb와 그래픽 드라이버의 오탐 때문에 꺼 두었습니다. |
| clang-tidy | Ubuntu / Clang | `src/meno/`의 구현을 정적 분석합니다. 지금은 경고만 내고 실패시키지 않습니다. |
| Doxygen 레퍼런스 | Ubuntu | Doxygen 1.15.0으로 API 레퍼런스와 사용자 설명서를 빌드합니다. 경고가 하나라도 있으면 실패하고, `main`에서는 GitHub Pages에 배포합니다. |

## 테스트 추가하기

테스트는 모듈별 폴더(`tests/<모듈>/`)에 `test_<이름>.cpp`로 두고, 같은 폴더의 `CMakeLists.txt`에서
`meno_add_test(<이름>)`으로 등록합니다. 등록하지 않은 파일이 있으면 configure가 실패합니다. 테스트
이름은 폴더가 달라도 겹쳐서는 안 됩니다.

- 앞에서 본 세 빌드 방식 가운데 가능한 한 위쪽 방식을 고릅니다. 방식별 등록 방법은
  [tests/CMakeLists.txt](https://github.com/SeoulTech-Fiat-Lux/Meno/blob/main/tests/CMakeLists.txt)의
  주석에 있습니다.
- 창을 여는 테스트는 `main`의 첫 줄에서 `meno::test::skipWithoutDisplay()`를 호출해 결과를 확인하고,
  등록할 때 `meno_display_test(<이름>)`을 덧붙입니다. `meno_display_test()`를 빠뜨리면 창 테스트를
  건너뛸 때 실패로 보고됩니다.
