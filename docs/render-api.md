# 2D 렌더 API 설계 노트

담당: 2D 렌더 API · 대상 표준: SFML 3.1.0 / C++20 / CMake 3.28

---

## 1. 이 계층이 지키는 규칙

**공개 헤더에는 SFML이 없다.** `include/meno/` 아래 어떤 파일도 `<SFML/...>`을 include하지
않는다. 사용자 프로젝트는 SFML 헤더 경로 없이 `meno`만 링크하면 빌드된다.

이걸 유지하는 장치가 세 개다.

| 장치 | 위치 | 역할 |
|---|---|---|
| pimpl | `Texture`, `Font`, `Renderer`, `Window` | SFML 객체를 `unique_ptr<Impl>` 뒤로 숨긴다 |
| `backend::*Access` | `src/meno/sfml_backend/Access.hpp` | 백엔드끼리만 내부 핸들을 꺼내는 유일한 통로 (friend) |
| `target_link_libraries(meno PRIVATE SFML::Graphics)` | `CMakeLists.txt` | SFML 헤더 경로가 사용자 타깃에 전파되지 않게 한다 |

셋 중 하나라도 빠지면 경계가 새기 때문에, 리뷰할 때 이 세 가지를 확인하면 된다.

검증 방법 — 아래가 아무것도 출력하지 않아야 한다. 공개 헤더가 SFML 타입을 얻을 수 있는
경로는 include뿐이므로, include만 막으면 경계는 지켜진다.
(`grep -r "SFML" include/`처럼 넓게 잡으면 설명 주석까지 걸려서 쓸모가 없다.)

```bash
grep -rn "#include.*SFML" include/
```

CI에 넣을 거면 이 명령이 매치되면 실패하도록 걸어두면 된다.

## 2. API 스타일

**즉시 모드 + 옵션 구조체.** 호출 순서가 곧 그리기 순서다(나중 호출이 위로).
커맨드 버퍼도 정렬 단계도 없다.

```cpp
renderer.beginFrame(meno::colors::Black);
renderer.draw(texture, {100.f, 50.f});                       // 가장 짧은 형태
renderer.draw(texture, {.position = {100.f, 50.f},           // 옵션 지정
                        .origin = {16.f, 16.f},
                        .rotation = 45.f,
                        .tint = meno::colors::Red});
renderer.endFrame();
```

`SpriteParams` / `ShapeParams` / `TextParams`는 전부 기본값을 가지므로 필요한 필드만 쓴다.

### 색상 리터럴

| 함수 | 받는 형태 | 예 |
|---|---|---|
| `Color::fromRgb` | 6자리 `0xRRGGBB` (불투명) | `Color::fromRgb(0x1E2430)` |
| `Color::fromRgba` | 8자리 `0xRRGGBBAA` | `Color::fromRgba(0x1E2430FF)` |

두 리터럴은 모두 `std::uint32_t`라서 오버로드로는 구분할 수 없다. 그래서 이름을 나눴다.
`fromRgba`에 6자리를 넣으면 `0x001E2430`으로 읽혀 색이 한 칸씩 밀리고 거의 투명해진다.
두 함수 모두 런타임 값을 받는다. 반대로 `fromRgb`에 6자리를 넘는 값을 넣으면 빌드 설정과 상관없이 이렇게 된다.

| 호출 | 결과 |
|---|---|
| 상수 식 (`constexpr` 변수, `static_assert`) | 컴파일 에러 |
| 그 밖 (함수 인자로 바로 넘기기, 런타임 값) | 마젠타(`0xFF00FF`). 게임은 멈추지 않는다 |

마젠타는 '잘못된 색'이라는 표시다. 색 하나 때문에 게임을 끄지 않고, 대신 화면에서 바로 눈에 띄게 한다.
컴파일할 때 확실히 잡고 싶은 색은 `constexpr` 변수로 받아 두자.

```cpp
constexpr meno::Color kSky = meno::Color::fromRgb(0x1E2430);  // 8자리를 넣으면 여기서 컴파일 에러
renderer.beginFrame(kSky);
```

> ⚠️ **반투명은 `withAlpha`로 만든다.** `fromRgb(0x00000080)`처럼 앞자리가 `00`인 8자리는 값이 6자리 범위라
> 어떤 검사로도 잡히지 않고 불투명한 남색(`0x000080`)이 된다. `meno::colors::Black.withAlpha(128)` 또는
> `fromRgba(0x00000080)`을 쓴다.

### 다각형과 부채꼴

```cpp
// 꼭짓점은 position 기준 로컬 좌표다. 모양은 한 번 만들어 두고 위치와 회전만 바꾼다.
constexpr std::array<meno::Vec2f, 3> kShip{{{14.f, 0.f}, {-10.f, -8.f}, {-10.f, 8.f}}};
renderer.drawPolygon(ship.position, kShip, {.fill = meno::colors::White, .rotation = ship.angle});

// 오른쪽을 보고 입을 60도 벌린 팩맨. 0도 = 오른쪽, 시계 방향.
renderer.drawSector(pac.position, 16.f, 30.f, 300.f, {.fill = meno::colors::Yellow});
```

- **`drawPolygon`의 점은 `position` 기준 로컬 좌표다.** `params.origin`(기본 `{0, 0}`)이 `position`에 놓이고 회전 중심이 된다.
  월드 좌표의 점을 그대로 그리려면 `position`을 `{0, 0}`으로 둔다. 이때 `rotation`은 월드 원점을 축으로 돈다.
- **채우기는 볼록 다각형만 보장한다.** SFML이 바운딩 박스 중심에서 삼각형 부채를 펴서 채우기 때문이다.
  그 중심에서 모든 변이 보이면 오목해도 맞게 채워진다. 중심 둘레로 꼭짓점을 돌리며 반지름만 흔든 운석은 대개 괜찮다.
  (꼭짓점 10개, 반지름을 절반까지 줄인 무작위 운석 2만 개를 시뮬레이션했을 때 깨진 것이 없었다.)
- **점이 3개 미만이면 그리지 않는다.** 중괄호 목록(`{{0.f, -12.f}, {9.f, 8.f}, {-9.f, 8.f}}`)은 `initializer_list`
  오버로드가 받는다. C++20의 `std::span`은 중괄호 목록을 받지 못한다.
- **`drawSector`는 180도가 넘어도 정확하다.** 실제 중심에서 삼각형 부채를 펴서 채운다. `sweepDeg`가 음수면 반시계로 펴고,
  절댓값이 360 이상이면 `drawCircle`과 같다. 호의 매끄러움은 `drawCircle`과 같다(한 바퀴 30변).
  외곽선은 꼭짓점에서 평평하게 끝나기 때문에 두께 밖으로 튀어나오지 않는다.
- **반지름이 0 이하(NaN, 무한대 포함)면 `drawCircle`과 `drawSector` 모두 그리지 않는다.**
  시간에 따라 줄어드는 반지름이 마지막 프레임에 음수가 되어도 원이 다시 나타나지 않는다.

### 텍스트 크기 재기

`measureText`는 같은 `TextParams`로 `drawText`했을 때의 크기(외곽선 포함)를 돌려준다.
다만 글자 잉크는 `drawText`의 position에서 바로 시작하지 않는다. 윗여백(글리프 베어링)과
외곽선만큼 어긋나 있으므로, 픽셀 단위로 정렬하려면 원점까지 주는 `measureTextBounds`를 쓴다.

```cpp
meno::TextParams params{.characterSize = 32};
params.origin = renderer.measureTextBounds(font, "GAME OVER", params).center();
renderer.drawText(font, "GAME OVER", screenCenter, params);  // 정확히 가운데. 회전해도 제자리
```

> ⚠️ **C++20 지정 초기화는 선언 순서를 지켜야 한다.** `SpriteParams`의 필드 순서를 바꾸면
> 사용자 코드가 컴파일 에러를 낸다. 필드 순서 변경 = API 변경으로 취급할 것. 새 필드는
> 항상 끝에 추가한다.

### 레이어가 필요하면

호출 순서로 표현한다. 배경 → 오브젝트 → UI 순으로 그리면 된다.
정렬 기반 레이어는 지금 넣지 않았다 — 필요해지면 `endFrame`에 정렬 단계를 추가하는 형태로
확장 가능하고, 그때 API는 `SpriteParams`에 `layer` 필드를 끝에 붙이면 된다.

## 3. 좌표계와 각도

- 원점은 좌상단, y축은 아래로 증가한다 (SFML/화면 좌표 관례).
- **각도는 전부 도(degree), 시계 방향.** 라디안은 공개 API에 등장하지 않는다.
- `origin`은 회전·스케일의 기준점이며 **스케일 적용 전 텍스처 픽셀 단위**다.
  텍스처 크기의 절반을 넣으면 제자리 회전이 된다.

카메라를 켠 상태에서 그린 것은 월드 좌표, `resetCamera()` 후에 그린 것은 화면 좌표다.
UI는 항상 `resetCamera()` 뒤에 그린다.

### 카메라 (`Camera2D`)

```cpp
renderer.setCamera({.center = player.position,   // 화면 가운데에 올 월드 좌표
                    .size = {320.f, 180.f},      // 설계 해상도: 창 크기와 상관없이 이만큼의 월드가 보인다
                    .zoom = 1.5f,                // 동적 확대: 보이는 영역 = size / zoom
                    .rotation = 10.f,            // 도, 시계 방향
                    .viewport = {0.f, 0.f, 1.f, 1.f},  // 창에서 그릴 영역(비율)
                    .fit = meno::CameraFit::Letterbox});  // 기본값. 꽉 채우려면 Stretch
```

| 필드 | 기본값 | 규칙 |
|---|---|---|
| `center` | `{0, 0}` | 뷰포트 가운데에 올 월드 좌표 |
| `size` | `{0, 0}` | 보일 월드 크기. `{0, 0}`이면 **뷰포트의 픽셀 크기**를 써서 1픽셀 = 1월드유닛 |
| `zoom` | `1` | `size` 위에 곱하는 배율. 0 이하·NaN이면 1 |
| `rotation` | `0` | 도, 시계 방향. 범위 제한 없음(내부에서 360으로 접는다) |
| `viewport` | `{0, 0, 1, 1}` | 창 전체를 1로 보는 비율 좌표. 분할 화면·미니맵 |
| `fit` | `Letterbox` | `size`와 뷰포트의 종횡비가 다를 때. 기본은 비율을 지키고 남는 띠를 비운다. `Stretch`면 찌그러지더라도 꽉 채운다 |

- **`size`와 `zoom`의 역할 분담.** `size`는 게임 내내 고정하는 설계 해상도, `zoom`은 연출용 확대다.
  둘을 동시에 바꾸지 말 것. 보이는 영역은 항상 `size / zoom`이다.
- **회전 방향.** 카메라를 시계 방향으로 돌리면 화면 속 월드는 **반시계 방향**으로 돈다.
  같은 `+45`라도 스프라이트(`SpriteParams::rotation`)는 시계 방향으로 돈다. 화면 위쪽이 가리키는 월드 방향은
  `0° → -y`, `90° → +x`, `180° → +y`, `-90° → -x`다. (y축은 아래로 증가하므로 회전 없을 때 위쪽은 `-y`)
- **`size`가 `{0, 0}`일 때의 "1픽셀 = 1월드유닛"은 뷰포트 기준이다.** 그런 카메라를 창의 왼쪽 절반에 두면
  보이는 영역은 창 크기가 아니라 절반 크기다. 창 크기를 쓰면 반쪽 화면에 창 전체가 들어가 가로로 찌그러진다.
- **뷰포트 픽셀은 네 모서리를 각각 반올림한다.** SFML은 위치와 크기를 따로 반올림해서, 창을 3등분하면
  화면 사이에 1픽셀 틈이 생긴다. 모서리를 반올림하면 맞닿은 뷰포트가 같은 경계를 공유한다.
- **뷰포트 밖으로는 그려지지 않는다.** 아주 큰 사각형을 그려서 미니맵 배경을 칠할 수 있다.
  레터박스 띠와 뷰포트 바깥은 `beginFrame`의 `clearColor`로 남는다.
- **`fit`의 기본값은 `Letterbox`다.** `size`를 정했다는 것은 "이 해상도로 설계했다"는 뜻이므로, 따로 지시하지
  않아도 비율을 지킨다. SFML의 `sf::View`처럼 꽉 채우려면 `.fit = meno::CameraFit::Stretch`를 명시한다.
  `size`가 `{0, 0}`인 기본 카메라는 맞출 비율이 없으므로 어느 쪽이든 같다.
  (Godot도 같은 구조다. 기본은 늘이지 않고(`stretch/mode = disabled`), 기준 해상도를 켜면 비율 유지(`stretch/aspect = keep`)가 기본이다.)
- **레터박스 띠 위의 마우스.** `screenToWorld`는 띠 위의 픽셀에도 같은 규칙을 연장해 보이는 영역 밖의 월드 좌표를
  돌려준다. 걸러야 하면 `meno::viewportPixels(camera, renderer.framebufferSize()).contains(mouse)`로 먼저 확인한다.

  화면 분할 예:

```cpp
renderer.setCamera({.center = p1.position, .viewport = {0.f, 0.f, 0.5f, 1.f}});
drawWorld(renderer);
renderer.setCamera({.center = p2.position, .viewport = {0.5f, 0.f, 0.5f, 1.f}});
drawWorld(renderer);
renderer.resetCamera();  // UI
```

### 카메라와 창 크기

**`Renderer`는 마지막 카메라를 기억했다가 `beginFrame`마다 지금 창 크기로 다시 적용한다.**
그래서 창 크기가 바뀌어도 게임 코드가 할 일은 없고, 카메라를 매 프레임 다시 `setCamera`할 필요도 없다.

- `resetCamera()`의 화면 좌표도 창 크기를 따라간다. 원점은 언제나 창 좌상단, 1픽셀 = 1월드유닛이다.
  (SFML의 `getDefaultView()`는 창을 만들 때 한 번만 정해져서, 그대로 쓰면 창을 키울 때 UI가 함께 늘어난다.)
- `size`가 `{0, 0}`인 카메라는 창이 커지면 월드가 더 넓게 보인다. 창 크기와 상관없이 같은 영역을 보이려면 `size`를 정해야 한다. 비율은 기본값(`Letterbox`)이 지켜 준다.
- 지금 카메라는 `renderer.camera()`로 읽는다. `resetCamera()` 상태(처음 포함)면 `std::nullopt`다.

### 좌표 변환은 그리기와 같은 계산을 쓴다

`worldToScreen` / `screenToWorld` / `visibleWorldBounds`의 계산은 `include/meno/math/Camera2D.hpp`의
같은 이름 자유 함수에 있다. `Renderer`의 멤버는 지금 카메라와 지금 창 크기를 넣어 부르기만 한다.
그리는 쪽도 `backend::toSfView`가 이 함수들로 `sf::View`를 만들기 때문에, 화면에 그려진 위치와 계산 결과가
일치한다. (`tests/graphics/test_camera_parity.cpp`가 무작위 카메라 3000개로 SFML과 대조한다.)

- 결과는 **정수로 자르지 않은 실수**다. 마우스 픽셀 좌표를 넣으면 그 픽셀의 좌상단 모서리에 해당하는 월드 좌표가 나온다.
- `Renderer` 없이도 쓸 수 있다. 창 없이 돌아가는 게임 로직이나 테스트에서
  `meno::screenToWorld(camera, framebufferSize, mouse)`처럼 부르면 된다.
- 주의: `Renderer`의 멤버 함수 안에서 이 자유 함수를 부를 때는 `meno::`를 붙여야 한다.
  이름이 같아서 그냥 부르면 멤버 자신이 불린다.

## 4. 다른 담당자와의 경계

### ResourceManager 담당자에게

`Texture`와 `Font`는 이 계층에서 정의한다. **캐싱과 재사용은 ResourceManager의 몫**이다.

- 로딩은 `static std::optional<Texture> Texture::loadFromFile(path)` 하나로 통일했다.
  실패는 예외가 아니라 빈 optional이다.
- 둘 다 **복사 불가, 이동 가능**이다. `ResourceManager`는 로드한 객체를
  `std::shared_ptr<const Texture>` / `std::shared_ptr<const Font>`로 보관한다.
- 로드 함수는 성공 시 공유 포인터, 실패 시 빈 공유 포인터를 반환한다. 같은 정규화
  경로와 옵션을 다시 요청하면 기존 객체를 반환하고 실패 결과는 캐시하지 않는다.
  텍스처 보간 방식은 `loadTexture(path, {.smooth = true})`처럼 로드 시 지정하며,
  `smooth` 값이 다르면 별도의 텍스처로 캐시한다.
- `Renderer`는 `const Texture&`만 받고 수명에 관여하지 않는다. 텍스처가 살아 있는지는
  호출자가 보장한다. `Sprite`는 공유 포인터를 보관하므로 매니저가 먼저 소멸해도 안전하다.
- 이동당한(moved-from) 텍스처를 그리면 조용히 건너뛴다. 크래시는 나지 않는다.
  그리기뿐 아니라 조회도 마찬가지다. `size()`는 `{0, 0}`, `isSmooth()`는 `false`를 돌려주고
  `setSmooth()`는 무시된다. 이동당한 폰트로 그리면 건너뛰고, 재면 `{0, 0}`이 나온다.

`Renderer`는 반대로 **복사도 이동도 불가**다. Window를 빌려 쓰는 타입이라 옮길 이유가 없고,
막아 두면 "살아 있는 Renderer는 언제나 그릴 수 있다"가 타입으로 보장된다. 여러 곳에서 쓰려면
`Renderer&`로 넘기고, 생성을 미뤄야 하면 `std::optional<Renderer>::emplace`나
`std::unique_ptr<Renderer>`를 쓴다.

### 충돌 판정 담당자에게

**`meno::Rect<T>`를 쓰자.** 별도의 AABB 타입을 새로 만들면 렌더와 충돌 사이에
변환 코드가 계속 생긴다. `contains` / `intersects` / `findIntersection`은 이미 있다.
필요한 연산이 더 있으면 `include/meno/math/Rect.hpp`에 추가해줘.

`Vec2<T>`도 마찬가지로 공용 타입이다. 정수 벡터를 쓸 때 알아둘 규칙이 셋 있다:

- **`length()`는 실수를 돌려준다.** `Vec2i`여도 `float`다. `Vec2i{1,1}.length()`는 1이 아니라
  1.414...다. 정수로 잘리면 조용히 틀린 거리가 나오므로 타입으로 막아뒀다.
- **`normalized()`는 `Vec2<RealOf<T>>`를 돌려준다.** `Vec2i`에 쓰면 `Vec2f`가 나온다.
  정수 나눗셈이면 `Vec2i{3,4}`의 방향이 `{0,0}`이 되어버리는데, 반환 타입을 바꿔서
  그 상황 자체가 생기지 않게 했다.
- **거리 비교·정렬에는 `lengthSquared()`를 쓰자.** `sqrt`를 피하고, 정수 벡터에서 정확하다.
  단 `Vec2i`는 성분이 약 46341을 넘으면 오버플로한다.

### Window 담당자에게

`Renderer`가 `Window`에게 요구하는 건 두 가지뿐이다:

1. `backend::WindowAccess::native()`로 꺼낼 수 있는 `sf::RenderWindow&`
2. **매 프레임 SFML 이벤트 큐를 비우는 것** (`pollEvents()`가 한다)

`Renderer`는 생성 시점에 위 1번의 렌더 타깃 주소를 붙잡아 둔다. 그래서 `Window`는 `Renderer`와 같이
**복사도 이동도 할 수 없다.** 이동을 허용하면, 다른 창을 이동 대입했을 때 붙잡은 주소가 허공을 가리킨다.
창을 다시 만드는 기능(전체 화면 전환 등)이 필요하면 `Window` 안의 `sf::RenderWindow`를 제자리에서
다시 만드는(`create()`) 방식으로 넣는다. 주소가 그대로라 `Renderer`를 다시 만들 필요가 없다.

`pollEvents()`는 닫기 버튼을 처리하고, 키보드·마우스 이벤트는 `Window`가 가진 `Input`에 넘긴다.
입력 쪽 규칙은 [input-api.md](input-api.md)에 있다.

## 5. SFML 3에서 바뀐 것들 (2.x 예제를 보고 짜면 틀리는 지점)

실제 SFML 3.1.0 헤더에서 확인한 내용이다. 2.x 기준 튜토리얼이 아직 검색 상위에 많아서
정리해 둔다.

| 항목 | SFML 2.x | SFML 3.1.0 |
|---|---|---|
| `sf::Sprite` 기본 생성자 | 있음 | **없음.** `Sprite(const Texture&)` 필수 |
| `sf::Text` 기본 생성자 | 있음 | **없음.** `Text(const Font&, String, unsigned)` |
| 폰트 로딩 | `loadFromFile` | **`openFromFile`** |
| `sf::Rect` 멤버 | `left/top/width/height` | **`position` / `size`** (Vector2) |
| `sf::VideoMode` | `VideoMode(w, h)` | **`VideoMode(Vector2u)`** |
| 이벤트 루프 | `while (window.pollEvent(e))` | **`while (const auto e = window.pollEvent())`** + `e->is<T>()` |
| 회전 각도 | `float` (도) | **`sf::Angle`** (`sf::degrees()` / `sf::radians()`) |
| CMake 타깃 | `sfml-graphics` | `sfml-graphics`도 여전히 유효, **`SFML::Graphics`가 권장/이식성 높음** |
| CMake 최소 버전 | 3.16 | **3.28** (SFML 최상단 CMakeLists 기준) |

### UTF-8 주의

`sf::String`에 `std::string`을 그냥 넘기면 **Latin-1로 해석해서 한글이 깨진다.**
`sf::String::fromUtf8(begin, end)`를 써야 한다. `Renderer::drawText`와 `Window` 제목이
이미 그렇게 처리하고 있으니, 백엔드에 문자열을 넘기는 코드를 새로 쓸 때만 주의하면 된다.

MSVC는 `/utf-8` 옵션을 줘야 한글 리터럴과 주석이 안전하다. CMakeLists에 이미 들어 있다.

## 6. 아직 안 한 것

의도적으로 뺐다. 필요해지면 그때 논의하자.

- 텍스처가 없을 때 마젠타 플레이스홀더 그리기 (지금은 조용히 건너뜀)
- 배치 렌더링 / 드로우콜 병합 — 즉시 모드라 스프라이트 하나당 드로우콜 하나다.
  W6 성능 벤치에서 병목으로 나오면 그때 `sf::VertexArray` 배칭을 넣는 게 순서다.
- 렌더 타깃(오프스크린), 셰이더, 블렌드 모드
- 텍스트 정렬(`TextAlign`) — 지금은 `measureTextBounds`로 직접 계산한다
- 오목 다각형 채우기(삼각분할) — 지금은 `drawPolygon`이 SFML처럼 볼록 다각형만 보장한다
