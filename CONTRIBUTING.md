# meno에 기여하기

## 빌드와 테스트

요구 사항과 Linux에서 먼저 설치할 패키지는 [README](README.md#시작하기)에 있다.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

> 첫 빌드는 SFML과 그 의존성(freetype·harfbuzz·SheenBidi)을
> 함께 빌드하기 때문에 조금 오래 걸린다.

## 저장소 구조

```
include/meno/     공개 헤더; 여기에 SFML은 등장하지 않는다
  Meno.hpp          통합 헤더
  math/             Vec2, Rect, Color, Angle, Camera2D
  core/             Application, Clock, Time
  input/            Input, Key, MouseButton, Event
  window/           Window
  graphics/         Renderer, Texture, Font, DrawParams
  audio/            SoundBuffer, Sound, Music
  resources/        ResourceManager
  scene/            Scene, SceneManager, GameObject, Component와 Transform·Sprite·Collider·AudioSource
  app/              Game
src/meno/         구현부; 사용자에게 배포되지 않는다. SFML은 여기서만 include한다
  sfml_backend/     SFML과 meno 타입 사이의 변환, 내부 접근 통로
examples/         예제 및 데모 게임
tests/            테스트 (consumer/는 하위 프로젝트 빌드 검사)
tools/            check_headers.py — 공개 헤더 SFML 유출 검사
docs/             설계 문서, API 레퍼런스 사이트의 첫 쪽·모듈 목록·테마
```

각 모듈은 위 목록에서 자기보다 위에 있는 모듈만 참조하며, 순환 의존 관계가 아니다.

`include/`와 `src/`가 나뉜 게 핵심이다. CMake에서 `include/`는 `PUBLIC`,
`src/`는 `PRIVATE`으로 지정해 **SFML이 사용자 빌드로 새지 않게** 한다.
자세한 근거는 [docs/render-api.md](docs/render-api.md)에 있다.

검사는 자동화되어 있다:

```bash
python tools/check_headers.py
```

## 문서 주석

공개 헤더의 `///` 주석이 [API 레퍼런스](https://seoultech-fiat-lux.github.io/Meno/)가 된다.
CI의 Docs 워크플로는 Doxygen 경고가 하나라도 있으면 실패하므로, 공개 헤더를 바꾸면 먼저 확인한다.

```bash
mkdir -p build-docs && doxygen Doxyfile 2> build-docs/warnings.txt
```

warnings.txt 맨 위의 "Korean ... has not been updated" 두 줄은 번역 안내라 경고가 아니다.
Doxygen은 CI와 같은 1.15.0을 쓴다.

- `///`는 합니다체로 쓰고, 사용자가 API를 쓰는 데 필요한 내용(동작, 계약, 주의할 점, 예제)만 둔다.
  설계 이유는 `///` 블록 바로 위의 `//`로, 계산 세부는 함수 본문의 `//`로 옮긴다.
  `//`는 한다체로 쓴다.
- 태그는 `\` 형식으로 쓰고, `\param`·`\return`은 시그니처만으로 알 수 없을 때만 쓴다.
- 클래스 블록은 요약 → 개념 → 주의할 점 → `\code{.cpp}` 예제 → `\see` 순서로 쓴다.
- 여러 멤버에 통용되는 주석을 붙일 때는 설명을 먼저 쓰고 그 블록의 마지막 줄에 `/// \{`를 둔다.
  `/// \}`로 닫고 그 뒤에 빈 줄을 둔다.
- private 멤버에는 `//`를 쓴다. `///` 예제 안에서는 `/* */`를 쓰지 않고, `///` 안에 짝 없는 백틱을
  쓰지 않는다.
- 새 공개 헤더는 namespace 본문을 `/// \addtogroup <폴더 이름>`과 `/// \{` ... `/// \}`로 감싸
  모듈에 싣는다([docs/groups.dox](docs/groups.dox) 참고).

## 작업 방식

작업은 이슈 단위로 브랜치를 따서 PR로 `main`에 병합한다.

- 브랜치: `<유형>/#<이슈 번호>-<설명>` (예: `Bugfix/#26-Project-Compatibility`)
- 커밋: `<유형>: <설명> #<이슈 번호>` (예: `fix: Ignore non-positive radius #35`)

`math/`는 렌더러와 충돌 판정이 함께 쓰는 공용 타입이므로, 바꿀 때는 양쪽의 리뷰를 거친다.

코드 스타일은 `.clang-format`을 따른다. 이 프로젝트에서는 **clang-format 21.1.7**을 사용한다.
저장 시 자동 포맷을 켜두면 된다.

```bash
pip install clang-format==21.1.7
```

저장소 전체를 포맷한 커밋은 `.git-blame-ignore-revs`에 모아 두었다.
클론한 뒤 한 번 설정하면 `git blame`이 이 커밋들을 건너뛴다.

```bash
git config blame.ignoreRevsFile .git-blame-ignore-revs
```
