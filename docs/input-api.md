# 입력 API 설계 노트

담당: 입력(#18) · 대상 표준: SFML 3.1.0 / C++20 · 범위: 키보드·마우스 (게임패드는 제외)

---

## 1. 구조

```
OS ─ sf::Event ─▶ Window::pollEvents ─▶ backend::fromSf ─▶ meno::Event ─▶ Input::handleEvent ─▶ 상태 표
                  (window/Window.cpp)   (sfml_backend/       (input/          (input/Input.cpp)
                                         InputConvert.cpp)    Event.hpp)
                                                                   게임 코드 ─▶ window.input().isDown(...)
```

| 파일 | 하는 일 | SFML |
|---|---|---|
| `include/meno/input/Key.hpp`, `MouseButton.hpp` | 키·버튼 열거형 | 없음 |
| `include/meno/input/Event.hpp` | SFML 없는 입력 이벤트(`std::variant`) | 없음 |
| `include/meno/input/Input.hpp`, `src/meno/input/Input.cpp` | 이벤트로 상태 표를 갱신하고 질의에 답한다 | 없음 |
| `src/meno/sfml_backend/InputConvert.*` | `sf::Event` → `meno::Event` 변환 | 있음 |
| `src/meno/window/Window.cpp` | SFML 이벤트를 꺼내 변환해 `Input`에 넘긴다 | 있음 |

- SFML을 만지는 곳은 `Window.cpp`와 `InputConvert.cpp` 두 곳뿐이다. `Input`은 SFML을 모른다.
  그래서 테스트는 창 없이 `meno::Event`를 직접 넣어 `Input`을 검증한다.
- `sf::Keyboard::isKeyPressed` 같은 실시간 조회는 쓰지 않는다.
  1. 두 프레임 사이에 눌렀다 뗀 키를 놓친다.
  2. 창 포커스와 상관없이 OS 전역 키 상태를 읽는다(다른 창에 타자 쳐도 캐릭터가 움직인다).
  3. 테스트에서 흉내 낼 수 없다.
- `Input`은 `Window::Impl` 안에 있다. Window를 이동 생성해도 `Input`의 주소가 바뀌지 않으므로
  미리 받아 둔 `Input&`가 계속 유효하다. 이동당한 Window의 `input()`은 `std::logic_error`를 던진다.
  (`Renderer`가 이동당한 Window를 받으면 `std::invalid_argument`를 던지는 것과 같은 취지다.)

## 2. 틱: "이번에 눌렸다"의 기준

`wasPressed`·`wasReleased`·`wasActionPressed`·`wasActionReleased`·`mouseWheelDelta`·`text`는
**이번 틱**에 일어난 일만 돌려준다. 틱은 질의한 위치에 따라 다르다.

| 질의 위치 | 틱 |
|---|---|
| `Application::update` 안 | fixed update 한 번 |
| 그 밖 (`processEvents`, `render`, 직접 짠 `while` 루프) | `pollEvents()`부터 다음 `pollEvents()` 전까지 |

이벤트가 들어오면 두 번호를 함께 적는다.

- 프레임 번호: `Input::beginFrame()`(= `pollEvents()`)마다 1씩 는다.
- update 번호: "이 이벤트를 처음 보게 될 update" = `Time`의 현재 단계 번호 + 1.

질의할 때 update 안이면 update 번호를, 밖이면 프레임 번호를 비교한다. 예 (fixedTimeStep 10ms):

| 프레임 | 이 프레임의 이벤트 | update 횟수 | update에서 `wasPressed(Space)` | render에서 |
|---|---|---|---|---|
| 0 | Space 누름 | 0 | — | true |
| 1 | — | 1 | **true** (프레임 0의 누름) | false |
| 2 | — | 3 | false, false, false | false |
| 3 | Space 뗌 | 3 | `wasReleased` true, false, false | `wasReleased` true |
| 4 | Space 눌렀다 뗌 | 1 | `wasPressed`·`wasReleased` 모두 true, `isDown` false | 둘 다 true |

update가 0번 도는 프레임에 들어온 입력은 다음 update로 넘어가고, 여러 번 도는 프레임에서는 첫 update만
본다. 휠과 텍스트도 같은 방식으로 "다음 update 한 번"에 모아서 전달한다(프레임 0·1의 글자 `a`·`b`는
update에서 `"ab"` 한 번으로 보인다).

`isDown`과 `mousePosition`은 틱과 상관없이 마지막 상태다.

## 3. 키보드·마우스 규칙

- **자동 반복은 무시한다.** 이미 눌린 키의 `KeyPressed`가 다시 와도 새 누름이 아니다.
- **누른 적 없는 키의 뗌은 무시한다.** 창이 열리기 전부터 누르고 있던 키 등.
- **`Key::Unknown`은 추적하지 않는다.** 서로 다른 여러 키가 한 값으로 모이기 때문이다.
- **포커스를 잃으면 눌린 것을 모두 뗀다.** 포커스가 없는 동안 뗀 키는 이벤트가 오지 않아
  "눌린 채"로 남기 때문이다. 그 틱에 `wasReleased`가 한 번 뜬다. 키를 누른 채로 돌아와
  `KeyPressed`가 오면 새 누름으로 본다.
- **`Key`는 자판 배열 기준이다.** `Key::A`는 A가 찍힌 키다. 물리적 위치(스캔코드)는 아직 없다.
  AZERTY 사용자를 위해 WASD를 위치 기준으로 받으려면 이후에 `Scancode`를 따로 추가해야 한다.
- **`mousePosition()`은 창 좌상단 기준 픽셀이다.** 카메라를 반영하지 않으므로 월드 좌표가 필요하면
  `Renderer::screenToWorld`로 바꾼다.
- **휠은 세로만 다룬다.** 위로 굴리면 양수, 한 틱 동안의 합이다.
- 범위를 벗어난 enum 값(`static_cast<Key>(250)`)은 무시하고 `false`를 돌려준다.

## 4. 텍스트 입력

- `text()`는 이번 틱에 입력된 글자를 UTF-8 `std::string` **복사본**으로 돌려준다. 다음 틱까지 들고 있어도 안전하다.
- 백스페이스·엔터·탭·Esc·Delete 같은 제어 문자는 빠진다. 이런 키는 `wasPressed(Key::Backspace)`처럼 키로 처리한다.
- **이동 키로 친 글자도 들어온다.** W를 눌러 앞으로 가면 `text()`에도 `"w"`가 온다. 글자를 받을 때(채팅창이
  열렸을 때 등)와 조작할 때는 게임이 나눠야 한다. `examples/input_demo`는 Enter로 입력 모드를 켜고 끈다.
- IME 조합 중인 글자는 오지 않고, 조합이 끝난 글자만 온다(SFML 제약).

## 5. 액션 매핑

```cpp
input.bind("jump", meno::Key::Space);
input.bind("jump", meno::Key::W);
input.bind("jump", meno::MouseButton::Left);

if (input.wasActionPressed("jump")) { ... }
```

| 규칙 | 이유 |
|---|---|
| 묶인 것 중 **하나라도** 눌려 있으면 `isActionDown` | 키보드·마우스를 동시에 지원 |
| `wasActionPressed`는 액션이 **안 눌림 → 눌림**으로 바뀐 틱에만 true | Space를 누른 채 W를 더 눌러도 점프가 두 번 나가지 않게 |
| `wasActionReleased`는 묶인 것이 **모두** 떼어진 틱에만 true | 위와 대칭 |
| 이미 눌린 키를 `bind`하면 `isActionDown`은 바로 true, `wasActionPressed`는 false | 이번 틱에 누른 것이 아니다 |
| 같은 키를 두 번 묶어도 한 번 | 키 재설정 UI가 반복 호출해도 목록이 늘지 않게 |
| 키 하나를 여러 액션이 같이 쓸 수 있다 | Esc = "pause"이면서 "back" |
| `unbind(name)`은 액션을 통째로 지운다 | 다시 `bind`하면 현재 키 상태에서 새로 시작 |
| 등록하지 않은 이름은 `false` | 예외 대신. 단, 이름 오타는 잡지 못한다 |

"안 눌림 → 눌림"을 키가 아니라 액션 단위로 판정해야 하므로, 액션은 매번 계산하지 않고 상태(`down`,
눌린 시점, 뗀 시점)를 들고 있다. 키·버튼 상태가 바뀔 때마다 `refreshActions`가 모든 액션을 다시 계산한다.

## 6. Time·Application과의 계약

- `Time`의 비공개 값 두 개를 내부 전용 `detail::TimeAccess`(`src/meno/core/TimeAccess.hpp`)가 읽기 전용으로 연다. `Input`은 이것만 쓴다.
  - `stepNumber()`: 지금까지 시작한 update 수. `beginRun`에서도 0으로 돌리지 않는다.
    이전 `run`에서 찍힌 번호와 새 `run`의 번호가 겹치지 않게 하려는 것이다.
  - `isInStep()`: 지금 update 안인가.
- `Application::run`은 `advanceStep` → `update` → `endStep` 순으로 호출한다.
  `update`가 예외를 던져도 `RunGuard`가 `endStep`을 불러 "update 안" 표시가 남지 않는다.
- `Application`을 쓰지 않는 루프에서는 `inStep_`이 항상 false이므로 모든 질의가 프레임 기준이다.

## 7. 범위 밖

- 게임패드
- 스캔코드(물리 위치 기준 키), 가로 휠
- 커서 숨기기·가두기, 상대 이동(`MouseMovedRaw`)
- 창 이벤트(`Resized` 등)를 사용자에게 노출. 지금 `meno::Event`는 입력 이벤트만 담는다.

## 8. 테스트

| 테스트 | 검사 | SFML | 창 |
|---|---|---|---|
| `tests/input/test_input.cpp` | 키·마우스·휠·텍스트·포커스·액션, update 기준 판정(스크립트된 Clock으로 0·1·3회 update) | 없음 | 없음 |
| `tests/input/test_input_convert.cpp` | SFML 키 101개가 서로 다른 meno 키로 빠짐없이 바뀌는지, 이벤트 변환 | 링크 | 없음 |
| `tests/window/test_window_input.cpp` | Window 이동 후에도 같은 `Input`, 이동당한 Window의 예외 | 링크 | 필요 (`MENO_DISPLAY_TESTS=1`) |
