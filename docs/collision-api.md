# 충돌 검사 사용법

게임 객체에 콜라이더를 추가한 뒤 `meno::intersects(a, b)`를 호출하면 두 객체가
겹치는지 확인할 수 있다. 다음 예제들은 `#include <meno/Meno.hpp>`를 사용하며,
`scene`은 게임에서 사용하는 `meno::Scene` 객체라고 가정한다.

## 1. 객체에 충돌 영역 붙이기

### 사각형 영역

`BoxCollider`에 객체 기준 위치(`offset`)와 크기(`size`)를 전달한다.
`offset`은 객체의 로컬 좌상단에서 충돌 영역의 좌상단까지의 거리다.

```cpp
auto& player = scene.createGameObject();
player.transform().pos = {100.f, 200.f};

auto& playerBox = player.addComponent<meno::BoxCollider>(
    meno::Vec2f{5.f, 10.f},   // 객체 기준 충돌 영역의 좌상단
    meno::Vec2f{30.f, 40.f}  // 충돌 영역의 가로, 세로
);
```

회전과 배율을 바꾸지 않았다면 이 영역의 월드 좌상단은 `(105, 210)`,
우하단은 `(135, 250)`이다. 이미지나 `Transform::width`, `height`를 설정해도
콜라이더가 자동으로 생기거나 크기가 맞춰지지 않으므로 직접 지정해야 한다.

### 원형 영역

`CircleCollider`에는 객체 기준 원의 **중심 위치**와 반지름을 전달한다.

```cpp
auto& enemy = scene.createGameObject();
enemy.transform().pos = {120.f, 220.f};

enemy.addComponent<meno::CircleCollider>(
    meno::Vec2f{0.f, 0.f}, // 원 중심을 객체 위치에 둔다
    10.f                  // 반지름
);
```

이 원의 월드 중심은 `(120, 220)`이다. 같은 `offset`이라도 사각형에서는 좌상단,
원에서는 중심을 뜻한다.

## 2. 두 객체가 겹치는지 검사하기

검사할 두 `GameObject`를 `meno::intersects()`에 전달한다.
사각형끼리, 원끼리, 사각형과 원 모두 같은 함수로 검사한다.

```cpp
if (meno::intersects(player, enemy)) {
    // 위 예제의 두 영역은 겹친다.
    // 피해 적용, 이동 취소 등 필요한 게임 처리를 여기서 수행한다.
}
```

충돌 검사는 게임의 `update()` 등 필요한 시점에 직접 호출한다. 객체 위치를 바꾼 뒤
호출하면 변경된 위치로 검사한다. 함수는 겹침 여부만 반환하며, 물체를 자동으로 밀어내거나
멈추게 하지 않는다.

예를 들어 이동 결과가 벽과 겹치면 이전 위치로 되돌릴 수 있다.

```cpp
// player와 wall에는 각각 콜라이더가 등록되어 있다고 가정한다.
const meno::Vec2f previousPosition = player.transform().pos;
player.transform().pos.x += 2.f;

if (meno::intersects(player, wall)) {
    player.transform().pos = previousPosition;
}
```

이 예제는 한 번의 이동을 통째로 취소하는 간단한 처리다. 검사는 호출한 순간의
영역만 비교하므로, 한 번에 벽을 완전히 넘어가는 이동까지 감지하지는 않는다.

## 3. 점이 사각형 안에 있는지 검사하기

등록한 `BoxCollider`의 `contains()`에 **월드 좌표**를 전달한다.

```cpp
if (playerBox.contains({120.f, 230.f})) {
    // 이 점은 플레이어의 충돌 영역 안에 있다.
}
```

마우스 위치를 검사할 때 카메라를 사용한다면 화면 좌표를 월드 좌표로 변환해서 넘긴다.

```cpp
const auto mouse = window.input().mousePosition();
const auto worldPoint = renderer.screenToWorld(
    {static_cast<float>(mouse.x), static_cast<float>(mouse.y)});

if (playerBox.contains(worldPoint)) {
    // 마우스가 플레이어의 사각형 충돌 영역 안에 있다.
}
```

이때 `renderer`에는 해당 객체를 그릴 때 사용하는 카메라가 설정되어 있어야 한다.
현재 `contains()`는 `BoxCollider`에서 제공한다.

## 4. 위치·회전·크기 변경하기

객체의 Transform을 변경하면 충돌 영역에도 적용된다. 콜라이더를 다시 등록할 필요가 없다.

```cpp
player.transform().pos = {300.f, 200.f};
player.transform().rotation = 45.f; // 도 단위, 시계 방향
player.transform().magnitude = 2.f; // 가로·세로를 함께 2배 확대
```

회전과 확대의 기준점은 객체의 로컬 원점이다. 따라서 충돌 영역의 크기뿐 아니라
`offset`도 함께 변환된다. 음수 배율은 원점 기준으로 반전하며, 크기와 반지름에는
배율의 절댓값을 적용한다.

충돌 영역 자체를 수정하려면 등록할 때 받은 참조를 사용한다.

```cpp
playerBox.offset = {8.f, 12.f};
playerBox.size = {24.f, 36.f};
```

한 객체에 `BoxCollider`와 `CircleCollider`를 하나씩 함께 등록할 수 있다.
두 객체의 콜라이더 조합 중 하나라도 겹치면 `intersects()`는 `true`를 반환한다.
같은 타입을 다시 `addComponent()`하면 기존 것이 교체되므로, 이전 콜라이더 참조는
더 이상 사용하면 안 된다.

## 사용 시 알아둘 규칙

| 상황 | 결과 또는 사용 방법 |
|---|---|
| 두 영역의 경계만 닿음 | `false`. 내부가 겹쳐야 충돌로 판정한다 |
| 점이 사각형의 변이나 꼭짓점 위에 있음 | `contains()`는 `false` |
| 검사할 콜라이더 조합이 없음 | `intersects()`는 `false` |
| 사각형의 가로·세로 또는 원의 반지름이 0 이하 | 해당 콜라이더는 충돌하지 않는다 |
| 배율이 0, 무한대 또는 NaN | 해당 콜라이더는 충돌하지 않는다 |
| `isTrigger` 설정 | 겹침 결과는 바뀌지 않고, 자동 이벤트도 발생하지 않는다 |
| `layer` 설정 | 자동 필터링되지 않는다. 검사할 대상을 게임 코드에서 선택한다 |

위치·회전·offset·크기·반지름에는 유한한 값을 사용한다.
콜라이더는 `addComponent()`로 객체에 등록해서 사용하며, 부모 객체가 없는 콜라이더는
충돌하지 않는다.

충돌이 시작된 순간에만 효과를 내고 싶다면 이전 업데이트의 결과를 저장해 두고,
`false`에서 `true`로 바뀌었을 때 처리한다. `intersects()` 자체는 겹쳐 있는 동안
호출할 때마다 `true`를 반환한다.
