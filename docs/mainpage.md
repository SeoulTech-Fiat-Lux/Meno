# meno 소개

meno는 SFML 3 위에서 2D 게임을 만드는 C++20 라이브러리입니다. 게임 루프, 씬과 게임 오브젝트,
충돌 판정, 리소스 관리처럼 게임마다 반복해서 짜는 부분을 제공하므로, SFML을 직접 다루지 않고 게임
규칙에 집중할 수 있습니다.

meno를 처음 쓴다면 \ref guide "사용자 설명서"의 1부부터 읽기를 권합니다. 클래스와 함수의
자세한 규칙은 아래 모듈 목록에서 찾을 수 있습니다.

## 짧은 예제

창을 띄우고, 스페이스를 누를 때마다 점수를 올려 화면에 그립니다.

```cpp
#include <meno/Meno.hpp>

#include <string>

int main() {
    meno::Game game{{.windowSize = {640, 480}, .title = "Hello meno"}};
    const auto font = game.resources().loadFont("assets/fonts/game.ttf");
    int score = 0;

    meno::Scene& play = game.scenes().createScene();
    play.setUpdateHandler([&](double) {
        if (game.window().input().wasPressed(meno::Key::Space)) {
            ++score;
        }
    });
    play.setDrawHandler([&](meno::Renderer& renderer) {
        if (font) {
            renderer.drawText(*font, "점수: " + std::to_string(score), {16.f, 16.f});
        }
    });
    (void)game.scenes().changeScene(play.id());
    game.run();
}
```

## 모듈

| 모듈 | 내용 |
|---|---|
| \ref app | 창, 렌더러, 리소스, 씬 루프를 한곳에 묶은 Game |
| \ref scene | Scene, GameObject, 컴포넌트, 충돌 판정 |
| \ref graphics | 텍스처, 도형, 텍스트 그리기와 카메라 |
| \ref input | 키보드와 마우스 입력, 동작(액션) 묶기 |
| \ref audio | 효과음과 배경음악 |
| \ref resources | 파일을 한 번만 읽어 공유하는 캐시 |
| \ref window | OS 창 하나와 그 창의 입력 |
| \ref core | 고정 시간 간격 게임 루프와 시간 |
| \ref math | 벡터, 사각형, 색, 각도, 2D 카메라 |
