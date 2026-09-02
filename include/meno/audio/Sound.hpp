#ifndef MENO_SOUND_HPP
#define MENO_SOUND_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

/// 메모리에 전체를 올려 재생하는 짧은 효과음.
///
/// 오디오 샘플과 재생 객체를 함께 소유한다. 복사할 수 없고 이동할 수 있으며,
/// 로딩 실패는 예외 대신 빈 optional로 알린다.
class Sound {
public:
    Sound();
    ~Sound();

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;
    Sound(Sound&&) noexcept;
    Sound& operator=(Sound&&) noexcept;

    /// 지원되는 오디오 파일을 메모리에 로드한다. 실패하면 std::nullopt.
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
