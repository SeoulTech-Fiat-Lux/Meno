#ifndef MENO_MUSIC_HPP
#define MENO_MUSIC_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

/// 파일에서 스트리밍해 재생하는 긴 배경음악.
///
/// 복사할 수 없고 이동할 수 있다. 재생 중에는 원본 파일을 이동하거나 삭제하면
/// 안 된다. 파일 열기 실패는 예외 대신 빈 optional로 알린다.
class Music {
public:
    Music();
    ~Music();

    Music(const Music&) = delete;
    Music& operator=(const Music&) = delete;
    Music(Music&&) noexcept;
    Music& operator=(Music&&) noexcept;

    /// 지원되는 오디오 파일을 스트리밍용으로 연다. 실패하면 std::nullopt.
    [[nodiscard]] static std::optional<Music> openFromFile(const std::filesystem::path& path);

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

#endif // MENO_MUSIC_HPP
