# 자동으로 이동하며 점프하는 상자

바닥과 나무 상자를 각각 `GameObject`로 만든 예제다. 외부 에셋이나 입력 없이
상자가 바닥 양 끝에서 방향을 바꾸고, 착지 후 잠깐 쉬었다가 다시 점프한다.
창을 닫으면 종료한다.

```bash
cmake -S . -B build
cmake --build build --target meno_example_jumping_box
./build/examples/meno_example_jumping_box
```

- `Game`이 고정 timestep과 `SceneManager::update()` 호출을 담당한다.
- Scene의 update handler가 수평 이동, 중력, 점프와 착지 반응을 담당한다.
- 두 객체 모두 기본 `Sprite`와 추가한 `BoxCollider`를 사용한다.
- 텍스처와 Collider 크기는 로컬 좌표이며, 공통으로 Transform의 4배 배율을 적용한다.
- `intersects()`는 충돌 여부만 반환한다. 내려오는 상자가 바닥과 겹치면 예제에서
  위치를 바닥 위로 보정하고 수직 속도를 0으로 만든다. 범용 물리 엔진은 아니다.
- 텍스처는 코드에서 생성하며, `Game`이 창을 파괴하기 전에 Scene을 정리한다.

움직임은 `main.cpp` 상단의 `speed`(수평 속도), `gravity`(중력),
`jumpSpeed`(점프 초기 속도), `jumpPause`(착지 후 대기 시간)로 조절한다.
