#ifndef MENO_AUDIO_HPP
#define MENO_AUDIO_HPP

#include <meno/audio/Sound.hpp>
#include <meno/scene/Component.hpp>

#include <filesystem>
#include <optional>

namespace meno {

/// GameObject에 붙이는 단일 효과음 재생 컴포넌트.
/// Sound를 독점 소유하며 교체/제거/소멸 시 기존 재생도 종료된다.
/// 기본 생성 시 음원과 오디오 장치를 생성하지 않는다.
class Audio final : public Component {
public:
    Audio() = default;
    explicit Audio(Sound&& sound);

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;
    Audio(Audio&&) = delete;
    Audio& operator=(Audio&&) = delete;

    /// 로딩 성공 시 교체한다. 실패하면 기존 음원과 재생 상태를 유지한다.
    [[nodiscard]] bool loadFromFile(const std::filesystem::path& path);
    /// 소유권을 넘겨받고 컴포넌트의 볼륨을 적용한다. 자동 재생하지 않는다.
    void setSound(Sound&& sound);
    void clearSound();
    /// Sound의 보유 여부. 전달받은 Sound의 로딩 성공 여부를 뜻하지 않는다.
    [[nodiscard]] bool hasSound() const noexcept;

    /// 일시정지 상태면 이어서 재생하고, 재생 중이면 처음부터 다시 재생한다.
    /// 음원이 없으면 재생 제어는 아무 작업도 하지 않는다.
    void play();
    void pause();
    void stop();
    [[nodiscard]] bool isPlaying() const;

    /// 0~1 범위로 제한하며 NaN은 무시한다. 음원을 바꿔도 설정은 유지된다.
    void setVolume(float volume);
    [[nodiscard]] float volume() const noexcept;

private:
    std::optional<Sound> sound_;
    float volume_{1.0F};
};

} // namespace meno

#endif // MENO_AUDIO_HPP
