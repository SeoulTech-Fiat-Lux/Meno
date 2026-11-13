# 8장. 오디오

이 장에서는 효과음과 배경음악을 재생합니다. meno의 오디오는 짧은 효과음용 `Sound`, 긴 배경음악용
`Music`, GameObject에 붙이는 `AudioSource` 컴포넌트로 나뉩니다.

## 완성 코드

Space를 누르면 효과음이 나고, M을 누르면 배경음악을 멈추거나 다시 재생합니다. 아래 두 파일이
필요하며, 파일이 없으면 그 소리만 나지 않습니다.

```
assets/sfx/jump.wav
assets/music/theme.ogg
```

```cpp
#include <meno/Meno.hpp>

#include <cstddef>
#include <optional>
#include <vector>

int main() {
    meno::Game game{{.application = {.framerateLimit = 60}, .title = "Audio"}};

    // 같은 효과음을 겹쳐 낼 수 있도록, 버퍼 하나를 공유하는 Sound를 네 개 둡니다.
    std::vector<meno::Sound> jumps;
    if (const auto buffer = game.resources().loadSoundBuffer("assets/sfx/jump.wav")) {
        for (int i = 0; i < 4; ++i) {
            jumps.emplace_back(buffer);
        }
    }
    std::size_t nextJump = 0;

    // Music은 재생하는 동안 살아 있어야 하므로 run()보다 오래 사는 곳에 둡니다.
    std::optional<meno::Music> theme = meno::Music::openFromFile("assets/music/theme.ogg");
    if (theme) {
        theme->setLooping(true);
        theme->setVolume(0.5f);
    }

    meno::Scene& scene = game.scenes().createScene();
    scene.setEnterHandler([&] {
        if (theme) {
            theme->play();
        }
    });
    scene.setExitHandler([&] {
        if (theme) {
            theme->stop();
        }
    });
    scene.setUpdateHandler([&](double) {
        const meno::Input& input = game.window().input();
        if (input.wasPressed(meno::Key::Space) && !jumps.empty()) {
            jumps[nextJump].play();
            nextJump = (nextJump + 1) % jumps.size();
        }
        if (input.wasPressed(meno::Key::M) && theme) {
            if (theme->isPlaying()) {
                theme->pause();
            } else {
                theme->play();
            }
        }
    });

    (void)game.scenes().changeScene(scene.id());
    game.run();
}
```

## Sound, SoundBuffer, Music

| 타입 | 용도 | 파일을 읽는 방식 |
|---|---|---|
| `SoundBuffer` | 효과음 데이터 | 파일 전체를 메모리에 올림 |
| `Sound` | 효과음 재생기 | `SoundBuffer`를 공유해 재생 |
| `Music` | 배경음악 | 재생하면서 파일을 조금씩 읽음 |

지원하는 파일 형식은 WAV, OGG, FLAC, MP3입니다.

효과음 데이터는 `game.resources().loadSoundBuffer()`로 읽습니다. 같은 경로를 다시 요청하면 처음 읽은
버퍼를 그대로 돌려주므로, 여러 `Sound`가 데이터 하나를 함께 씁니다. `Sound`는 버퍼를 함께 소유하므로
버퍼의 수명은 신경 쓰지 않아도 됩니다. 재생 위치와 볼륨은 `Sound`마다 따로 가집니다.

`Sound::loadFromFile()`과 `Music::openFromFile()`은 실패하면 `std::nullopt`를, `loadSoundBuffer()`는
빈 `shared_ptr`을 돌려줍니다. 모두 예외를 던지지 않으므로 쓰기 전에 확인합니다.

## 재생 함수

`Sound`, `Music`, `AudioSource`는 같은 재생 함수를 가집니다.

| 함수 | 동작 |
|---|---|
| `play()` | 멈춰 있으면 처음부터, 일시정지 중이면 이어서, 재생 중이면 처음부터 다시 재생 |
| `pause()` | 일시정지. 다시 `play()`하면 이어서 재생 |
| `stop()` | 멈추고 재생 위치를 처음으로 되돌림 |
| `isPlaying()` | 재생 중이면 true. 일시정지나 정지 상태면 false |
| `setVolume(v)` | 볼륨을 0(무음)~1(최대)로 지정. 범위를 벗어난 값은 범위 안으로 자름 |

`Music::setLooping(true)`를 부르면 음악이 끝난 뒤 처음부터 다시 재생합니다. 기본으로는 반복하지
않으며, `Sound`에는 반복 재생 기능이 없습니다.

## 같은 효과음 겹쳐 내기

`play()`는 이미 재생 중인 소리를 처음부터 다시 시작합니다. 그래서 `Sound` 하나로는 같은 효과음을
겹쳐 낼 수 없습니다. 연달아 누르는 점프나 총소리처럼 소리가 겹쳐야 한다면, 위 예제처럼 같은 버퍼를
쓰는 `Sound`를 여러 개 두고 차례로 재생합니다.

## GameObject에 소리 붙이기

GameObject마다 소리를 두려면 `AudioSource` 컴포넌트를 붙입니다. `AudioSource`는 `Sound` 하나를
소유하며, GameObject가 사라지면 재생 중이던 소리도 멈춥니다.

```cpp
const auto hit = game.resources().loadSoundBuffer("assets/sfx/hit.wav");

meno::GameObject& enemy = play.createGameObject();
auto& audio = enemy.addComponent<meno::AudioSource>();
audio.setSound(meno::Sound{hit});
audio.setVolume(0.8f);

// 적이 맞았을 때
audio.play();
```

볼륨은 `AudioSource`가 기억하므로 `setSound()`로 음원을 바꿔도 유지됩니다.
`AudioSource::loadFromFile()`도 있지만 부를 때마다 파일을 새로 읽으므로, 같은 소리를 여러
GameObject에서 쓴다면 위처럼 `loadSoundBuffer()`로 읽은 버퍼를 공유합니다.

## 주의할 점

- `Sound`와 `Music`은 소멸하는 순간 소리도 멈춥니다. 재생하는 동안 살아 있는 곳(게임 클래스의 멤버,
  `run()`보다 오래 사는 변수 등)에 둡니다. 함수 안의 지역 변수로 만들어 `play()`하면 함수가 끝날 때
  바로 멈춥니다.
- `Music`은 재생하면서 파일을 읽으므로, `Music`이 살아 있는 동안 원본 파일을 옮기거나 지우면 안
  됩니다.
- `Sound::loadFromFile()`은 부를 때마다 파일을 새로 읽습니다. 같은 효과음을 여러 곳에서 쓴다면
  `loadSoundBuffer()`로 버퍼를 공유합니다.

## 관련 예제와 API

- API: [Sound](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_sound.html),
  [SoundBuffer](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_sound_buffer.html),
  [Music](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_music.html),
  [AudioSource](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_audio_source.html),
  [ResourceManager](https://seoultech-fiat-lux.github.io/Meno/classmeno_1_1_resource_manager.html)

이로써 2부를 마칩니다. GameObject와 컴포넌트, 충돌 판정, 여러 Scene, 소리를 조합하면 슈팅, 스네이크,
플래피 버드 같은 게임을 만들 수 있습니다.

이전 장: [7장. 씬 전환과 씬 상태](07-scenes.md) · 다음 장: [9장. 리소스 관리](09-resources.md)
