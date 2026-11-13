#ifndef MENO_MUSIC_HPP
#define MENO_MUSIC_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

/// 파일에서 조금씩 읽어 재생하는 긴 배경음악입니다. 짧은 효과음은 Sound를 씁니다.
///
/// 복사할 수 없고 이동할 수 있습니다. 파일 열기 실패는 예외가 아니라 빈 optional로 알립니다.
/// 열지 않았거나 이동된 Music은 재생 함수를 불러도 아무 소리도 내지 않습니다.
///
/// 주의할 점:
/// - Music이 소멸하면 소리도 멈춥니다. 지역 변수로 만들어 play()하면 함수가 끝날 때 바로
///   멈추므로, 게임 클래스의 멤버처럼 재생하는 동안 살아 있는 곳에 둡니다.
/// - 파일을 재생하면서 읽으므로, Music이 살아 있는 동안 원본 파일을 옮기거나 지우면 안 됩니다.
///
/// \code{.cpp}
/// std::optional<meno::Music> bgm = meno::Music::openFromFile("assets/bgm.ogg");
/// if (bgm) {
///     bgm->setLooping(true);
///     bgm->setVolume(0.5f);
///     bgm->play();
/// }
/// \endcode
///
/// \see Sound
class Music {
public:
    /// 빈 Music을 만듭니다. 이 Music으로는 아무 소리도 나지 않습니다.
    Music();
    ~Music();

    Music(const Music&) = delete;
    Music& operator=(const Music&) = delete;
    /// 이동합니다. 이동된 쪽은 아무 소리도 내지 않으며, 새 값을 대입하기 전에는 쓰지 않습니다.
    /// \{
    Music(Music&&) noexcept;
    Music& operator=(Music&&) noexcept;
    /// \}

    /// 오디오 파일(WAV, OGG, FLAC, MP3)을 엽니다. 실패하면 std::nullopt를 돌려줍니다.
    ///
    /// 파일 전체를 미리 읽지 않고 재생하면서 읽습니다. 볼륨은 1, 반복은 꺼진 상태로 시작합니다.
    [[nodiscard]] static std::optional<Music> openFromFile(const std::filesystem::path& path);

    /// 재생합니다. 멈춰 있으면 처음부터, 일시정지 중이면 이어서, 이미 재생 중이면 처음부터 다시
    /// 재생합니다.
    void play();
    /// 일시정지합니다. play()를 부르면 이어서 재생합니다.
    void pause();
    /// 멈추고 재생 위치를 처음으로 되돌립니다.
    void stop();

    /// true면 음악이 끝난 뒤 처음부터 다시 재생합니다. 기본값은 false입니다.
    /// \{
    void setLooping(bool looping);
    [[nodiscard]] bool isLooping() const;
    /// \}

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

} // namespace meno

#endif // MENO_MUSIC_HPP
