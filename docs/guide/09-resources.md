# 9장. 리소스 관리

이 장에서는 `ResourceManager`가 텍스처, 폰트, 효과음 파일을 어떻게 읽고 공유하는지 정리하고, 게임에
필요한 파일을 시작할 때 한 번에 읽어 확인하는 방법을 다룹니다.

## 완성 코드

게임을 시작하기 전에 필요한 파일을 모두 읽고, 하나라도 없으면 빠진 파일을 알린 뒤 끝냅니다. 파일이
모두 있으면 그림과 글자를 그리고, Space를 누르면 효과음을 냅니다.

```cpp
#include <meno/Meno.hpp>

#include <iostream>
#include <string>
#include <vector>

int main() {
    meno::Game game{{.application = {.framerateLimit = 60}, .title = "Resources"}};
    meno::ResourceManager& resources = game.resources();

    const auto player = resources.loadTexture("assets/player.png");
    const auto font = resources.loadFont("assets/fonts/game.ttf");
    const auto jumpBuffer = resources.loadSoundBuffer("assets/sfx/jump.wav");

    // 읽지 못한 파일을 모아 알리고, 게임을 시작하지 않습니다.
    std::vector<std::string> missing;
    const auto require = [&](bool loaded, const char* path) {
        if (!loaded) {
            missing.emplace_back(path);
        }
    };
    require(player != nullptr, "assets/player.png");
    require(font != nullptr, "assets/fonts/game.ttf");
    require(jumpBuffer != nullptr, "assets/sfx/jump.wav");
    if (!missing.empty()) {
        for (const std::string& path : missing) {
            std::cerr << "Missing file: " << path << '\n';
        }
        return 1;
    }

    meno::Sound jump{jumpBuffer};
    meno::Scene& scene = game.scenes().createScene();
    scene.setUpdateHandler([&](double) {
        if (game.window().input().wasPressed(meno::Key::Space)) {
            jump.play();
        }
    });
    scene.setDrawHandler([&](meno::Renderer& renderer) {
        renderer.draw(*player, {.position = {100.f, 100.f}});
        renderer.drawText(*font, "Press Space", {100.f, 300.f});
    });

    (void)game.scenes().changeScene(scene.id());
    game.run();
}
```

## ResourceManager가 하는 일

`ResourceManager`는 파일 경로마다 리소스를 한 번만 읽어 두고 공유하는 캐시입니다. `Game`을 쓰면
`game.resources()`로 얻습니다.

| 함수 | 돌려주는 것 |
|---|---|
| `loadTexture(path, options)` | `std::shared_ptr<const Texture>` |
| `loadFont(path)` | `std::shared_ptr<const Font>` |
| `loadSoundBuffer(path)` | `std::shared_ptr<const SoundBuffer>` |

- 같은 경로를 다시 요청하면 처음 읽은 것과 같은 `shared_ptr`을 돌려줍니다. 여러 GameObject가 같은
  텍스처를 써도 파일은 한 번만 읽힙니다.
- 읽지 못하면 빈 `shared_ptr`을 돌려줍니다. 실패는 캐시하지 않으므로, 파일을 고친 뒤 다시 요청하면
  다시 읽습니다.
- 돌려받은 리소스는 `const`입니다. 텍스처 보간은 `setSmooth()` 대신 `loadTexture(path, {.smooth =
  true})`처럼 옵션으로 정하며, 옵션이 다르면 같은 경로라도 따로 읽습니다.

## 경로가 같은지 판단하는 방법

`ResourceManager`는 경로를 글자 단위로만 정리해 같은 파일인지 판단합니다. `./`와 `../`를 정리하고
경로 구분자를 통일하므로 `assets/./player.png`와 `assets/player.png`는 같은 리소스입니다. 하지만
다음 경우는 같은 파일이라도 따로 읽습니다.

- 상대 경로와 절대 경로
- 대소문자가 다른 경로. 대소문자는 실제 파일 이름과 똑같이 씁니다. Windows에서는 대소문자가 틀려도
  열리지만, Linux에서는 열리지 않습니다.

한 게임 안에서는 경로를 한 가지 방식으로 적는 것이 안전합니다. 상대 경로의 기준은 1장에서 설명한
현재 작업 디렉터리입니다.

## 리소스의 수명

한 번 읽은 리소스는 `ResourceManager`가 사라질 때까지 캐시에 남습니다. 캐시를 비우는 함수는
없으므로, 게임 도중에 큰 리소스를 많이 바꿔 읽는다면 메모리 사용량에 주의합니다.

돌려받은 `shared_ptr`을 가진 쪽이 있으면, `ResourceManager`가 사라진 뒤에도 그 리소스는 남아
있습니다. 반대로 캐시가 `Font`를 붙잡고 있는 동안에는 폰트 파일이 열린 채로 있으므로, 게임이
실행되는 동안 폰트 파일을 옮기거나 지우면 안 됩니다.

## ResourceManager를 거치지 않는 방법

각 리소스 타입에는 파일을 직접 읽는 함수도 있습니다. 이 함수들은 캐시를 쓰지 않으므로 부를 때마다
파일을 새로 읽습니다.

| 함수 | 돌려주는 것 | 쓰는 곳 |
|---|---|---|
| `Texture::loadFromFile(path)` | `std::optional<Texture>` | 한 곳에서만 쓰는 이미지 |
| `Font::loadFromFile(path)` | `std::optional<Font>` | 한 곳에서만 쓰는 폰트 |
| `SoundBuffer::loadFromFile(path)` | `std::optional<SoundBuffer>` | 한 곳에서만 쓰는 효과음 |
| `Music::openFromFile(path)` | `std::optional<Music>` | 배경음악(캐시하지 않음) |

`Music`은 재생하면서 파일을 읽으므로 `ResourceManager`가 다루지 않습니다. 배경음악은 8장처럼
`Music::openFromFile()`로 엽니다.

## 주의할 점

- 리소스를 쓰기 전에 항상 비었는지 확인합니다. 위 예제처럼 시작할 때 한 번에 확인해 두면 그 뒤에는
  확인을 생략할 수 있습니다.
- 매 프레임 `loadTexture()`를 불러도 캐시 덕분에 파일을 다시 읽지는 않지만, 경로를 정리하고 찾는
  비용은 매번 듭니다. 리소스는 시작할 때 읽어 변수에 받아 둡니다.
- `ResourceManager`는 여러 스레드에서 동시에 쓰면 안 됩니다.

## 관련 예제와 API

- API:
  [ResourceManager](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_resource_manager.html),
  [TextureOptions](https://seoultech-fiat-lux.github.io/Meno/structmeno_1_1_texture_options.html),
  [Texture](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_texture.html),
  [Font](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_font.html),
  [SoundBuffer](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_sound_buffer.html),
  [Music](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_music.html)

이전 장: [8장. 오디오](08-audio.md) · 다음 장: [10장. 카메라와 좌표](10-camera.md)
