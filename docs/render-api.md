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
| `Color::fromHex` | 8자리 `0xRRGGBBAA` | `Color::fromHex(0x1E2430FF)` |

두 리터럴은 모두 `std::uint32_t`라서 오버로드로는 구분할 수 없다. 그래서 이름을 나눴다.
`fromHex`에 6자리를 넣으면 `0x001E2430`으로 읽혀 색이 한 칸씩 밀리고 거의 투명해진다.
반대로 `fromRgb`에 8자리를 넣으면 디버그 빌드에서 assert로 멈춘다.

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

## 4. 다른 담당자와의 경계

### ResourceManager 담당자에게

`Texture`와 `Font`는 이 계층에서 정의한다. **캐싱과 재사용은 ResourceManager의 몫**이다.

- 로딩은 `static std::optional<Texture> Texture::loadFromFile(path)` 하나로 통일했다.
  실패는 예외가 아니라 빈 optional이다.
- 둘 다 **복사 불가, 이동 가능**이다. `std::unordered_map<std::string, Texture>`에
  그대로 담으면 된다.
- `Renderer`는 `const Texture&`만 받고 수명에 관여하지 않는다. 텍스처가 살아 있는지는
  호출자가 보장한다.
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

### Core(Window/Clock) 담당자에게

`include/meno/core/Window.hpp`와 `src/meno/core/Window.cpp`는 **내가 만든 임시 스텁**이다.
렌더 API를 지금 컴파일하고 눈으로 확인하려고 최소한만 넣었다. 실제 Window를 만들면
통째로 교체하면 된다.

`Renderer`가 `Window`에게 요구하는 건 두 가지뿐이다:

1. `backend::WindowAccess::native()`로 꺼낼 수 있는 `sf::RenderWindow*`
2. `size()`

이 둘만 유지되면 이벤트 큐든 Config 연동이든 자유롭게 설계해도 렌더 쪽은 영향받지 않는다.

`Renderer`는 생성 시점에 위 1번의 `sf::RenderWindow*`를 꺼내 붙잡아 둔다. 그래서 알아둘 점이 둘 있다.

- `native()`가 `nullptr`을 돌려주면(이동당한 Window) `Renderer` 생성자가
  `std::invalid_argument`를 던진다.
- Window에 다른 Window를 **이동 대입**하면 기존 렌더 타깃이 파괴되어 붙잡은 포인터가
  허공을 가리킨다. 창을 다시 만드는 기능(전체 화면 전환 등)을 넣는다면 Renderer도 함께
  다시 만들거나, Window를 이동 불가로 바꾸는 쪽을 검토해 달라.

현재 스텁의 `pollEvents()`는 닫기 버튼만 처리한다. 입력 처리는 Input 서비스 담당 영역이다.

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
