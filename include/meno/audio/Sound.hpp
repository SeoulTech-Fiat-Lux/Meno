#ifndef MENO_SOUND_HPP
#define MENO_SOUND_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

class SoundBuffer;

/// 메모리에 전체를 올려 재생하는 짧은 효과음.
///
/// SoundBuffer를 공유하고 독립된 재생 상태를 소유한다. 복사할 수 없고 이동할 수
/// 있으며, 로딩 실패는 예외 대신 빈 optional로 알린다.
class Sound {
public:
    Sound();
    explicit Sound(std::shared_ptr<const SoundBuffer> buffer);
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;
    Sound(Sound&&) noexcept;
    Sound& operator=(Sound&&) noexcept;

    /// 편의 함수. SoundBuffer를 로드해 이 Sound에 연결한다. 실패하면 std::nullopt.
    [[nodiscard]] static std::optional<Sound> loadFromFile(const std::filesystem::path& path);

    void play();
    void pause();
    void stop();

    /// 볼륨을 0(무음)~1(최대) 범위로 설정한다. 범위를 벗어난 값은 제한된다.
    void setVolume(float volume);
    [[nodiscard]] float volume() const;
    [[nodiscard]] bool isPlaying() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace meno

#endif // MENO_SOUND_HPP
