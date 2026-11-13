#ifndef MENO_SOUND_HPP
#define MENO_SOUND_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

/// \addtogroup audio
/// \{

class SoundBuffer;

/// 메모리에 올려 둔 짧은 효과음입니다. 긴 배경음악은 Music을 씁니다.
///
/// 소리 데이터(SoundBuffer)는 여러 Sound가 공유하고, 재생 위치와 볼륨은 Sound마다 따로
/// 가집니다. Sound가 SoundBuffer를 함께 소유하므로 버퍼의 수명은 신경 쓰지 않아도 됩니다.
/// 복사할 수 없고 이동할 수 있습니다.
///
/// 주의할 점:
/// - Sound가 소멸하면 소리도 멈춥니다. 재생하는 동안 살아 있는 곳에 둡니다.
/// - play()는 재생 중이면 처음부터 다시 시작합니다. 같은 효과음을 겹쳐 내려면 같은 버퍼를 쓰는
///   Sound를 여러 개 둡니다.
///
/// \code{.cpp}
/// const auto buffer = resources.loadSoundBuffer("assets/hit.wav");
/// meno::Sound first{buffer};
/// meno::Sound second{buffer};
/// first.play();
/// second.play(); // first와 겹쳐 들립니다.
/// \endcode
///
/// \see SoundBuffer, Music, AudioSource, ResourceManager::loadSoundBuffer
class Sound {
public:
    /// 빈 Sound를 만듭니다. 이 Sound로는 아무 소리도 나지 않습니다.
    Sound();
    /// buffer를 재생하는 Sound를 만듭니다. buffer가 비었거나 로드되지 않은 버퍼면 빈 Sound가
    /// 됩니다.
    explicit Sound(std::shared_ptr<const SoundBuffer> buffer);
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;
    /// 이동합니다. 이동된 쪽은 아무 소리도 내지 않으며, 새 값을 대입하기 전에는 쓰지 않습니다.
    /// \{
    Sound(Sound&&) noexcept;
    Sound& operator=(Sound&&) noexcept;
    /// \}

    /// 파일을 읽어 새 SoundBuffer를 만들고, 그 버퍼를 재생하는 Sound를 돌려줍니다. 실패하면
    /// std::nullopt를 돌려줍니다.
    ///
    /// 부를 때마다 파일을 새로 읽습니다. 같은 파일을 여러 Sound에서 쓰려면
    /// ResourceManager::loadSoundBuffer()로 버퍼를 공유합니다.
    [[nodiscard]] static std::optional<Sound> loadFromFile(const std::filesystem::path& path);

    /// 재생합니다. 멈춰 있으면 처음부터, 일시정지 중이면 이어서, 이미 재생 중이면 처음부터 다시
    /// 재생합니다.
    void play();
    /// 일시정지합니다. play()를 부르면 이어서 재생합니다.
    void pause();
    /// 멈추고 재생 위치를 처음으로 되돌립니다.
    void stop();

    /// 볼륨입니다. 0(무음)~1(최대)이며 기본값은 1입니다.
    ///
    /// 범위를 벗어난 값은 범위 안으로 자르고, NaN은 무시해 기존 값을 유지합니다.
    /// \{
    void setVolume(float volume);
    [[nodiscard]] float volume() const;
    /// \}

    /// 지금 재생 중이면 true입니다. 일시정지나 정지 상태면 false입니다.
    [[nodiscard]] bool isPlaying() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/// \}

} // namespace meno

#endif // MENO_SOUND_HPP
