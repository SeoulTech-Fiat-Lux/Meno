# 예제

meno의 기능을 직접 실행해 볼 수 있는 작은 프로그램입니다. 모든 예제는 에셋 파일 없이 실행됩니다.
텍스처는 코드로 만들고, 글자를 그리는 예제는 시스템 폰트를 찾아 쓰며 폰트가 없으면 글자만 그리지
않습니다.

| 예제 | 보여 주는 기능 | 관련 장 |
|---|---|---|
| [scene_transition](scene_transition/main.cpp) | 두 Scene을 3초마다 전환, enter·update·exit 핸들러 | [2장][ch2], [7장][ch7] |
| [render_basics](render_basics/main.cpp) | 도형·텍스트·스프라이트 그리기, 미니맵 카메라, `Game` 없이 구현한 루프 | [3장][ch3], [10장][ch10] |
| [input_demo](input_demo/main.cpp) | 키보드·마우스, 동작 이름 묶기, 휠, 글자 입력, `Application` 상속 | [4장][ch4], [12장][ch12] |
| [gameobject_demo](gameobject_demo/main.cpp) | GameObject 하나의 `Transform`과 `Sprite` 설정 | [5장][ch5] |
| [jumping_box](jumping_box/main.cpp) | `BoxCollider`와 `intersects()`로 착지 판정, 중력과 자동 점프 | [6장][ch6] |

8장(오디오), 9장(리소스 관리), 11장(스프라이트 애니메이션)에는 따로 예제가 없으며, 대신 각 장의
"완성 코드"를 실행해 볼 수 있습니다. 8장과 9장의 코드는 장에 적힌 에셋 파일이 있어야 제대로
동작합니다.

## 빌드와 실행

저장소 루트에서 빌드하면 예제도 함께 빌드됩니다. 실행 파일 이름은 `meno_example_<예제 이름>`입니다.

```bash
cmake -S . -B build
cmake --build build --target meno_example_jumping_box
./build/examples/meno_example_jumping_box
```

Visual Studio 생성기(Windows의 기본값)를 쓰면 실행 파일은 `build/examples/Debug/`에 생깁니다.
`MENO_BUILD_EXAMPLES` 옵션은 meno를 최상위 프로젝트로 빌드할 때만 기본으로 켜지므로, 다른
프로젝트에서 FetchContent로 가져온 meno의 예제는 빌드되지 않습니다.

## 예제 추가하기

`<예제 이름>/main.cpp`를 만들고 [CMakeLists.txt](CMakeLists.txt)에 `meno_add_example(<예제 이름>)`을
한 줄 추가합니다. 그리고 위 표에도 한 줄을 더합니다. 예제의 설명과 조작 방법은 따로 문서를 두지 않고
`main.cpp` 맨 위의 주석에 적습니다.

[ch2]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_202-first-game.html
[ch3]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_203-drawing.html
[ch4]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_204-input.html
[ch5]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_205-gameobjects.html
[ch6]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_206-collision.html
[ch7]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_207-scenes.html
[ch10]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_210-camera.html
[ch12]: https://seoultech-fiat-lux.github.io/Meno/md_docs_2guide_212-application.html
