#ifndef MENO_AUDIO_SOURCE_HPP
#define MENO_AUDIO_SOURCE_HPP

#include <meno/audio/Sound.hpp>
#include <meno/scene/Component.hpp>

#include <filesystem>
#include <optional>

namespace meno {

/// GameObject에 붙여 효과음 하나를 재생하는 컴포넌트입니다.
///
/// Sound 하나를 소유하므로, 음원을 바꾸거나 지우거나 GameObject가 사라지면 재생 중이던 소리도
/// 멈춥니다. 볼륨은 컴포넌트가 기억하므로 음원을 바꿔도 유지됩니다. 기본 생성자는 음원을 만들지
/// 않습니다.
///
/// 같은 효과음을 여러 GameObject에서 쓰면 loadFromFile()은 파일을 매번 새로 읽습니다. 이때는
/// ResourceManager::loadSoundBuffer()로 버퍼를 공유해 setSound()에 넘깁니다.
///
/// \code{.cpp}
/// auto& audio = enemy.addComponent<meno::AudioSource>();
/// audio.setSound(meno::Sound{resources.loadSoundBuffer("assets/hit.wav")});
/// audio.play();
/// \endcode
///
/// \see Sound, ResourceManager::loadSoundBuffer
class AudioSource final : public Component {
public:
    /// 음원 없는 AudioSource를 만듭니다.
    AudioSource() = default;
    /// sound를 넘겨받아 만듭니다. setSound()와 같습니다.
    explicit AudioSource(Sound&& sound);
    ~AudioSource() override = default;

    AudioSource(const AudioSource&) = delete;
    AudioSource& operator=(const AudioSource&) = delete;
    AudioSource(AudioSource&&) = delete;
    AudioSource& operator=(AudioSource&&) = delete;

    /// 파일을 읽어 음원을 바꿉니다. 성공하면 true입니다. 실패하면 기존 음원과 재생 상태를
    /// 그대로 두고 false를 돌려줍니다.
    [[nodiscard]] bool loadFromFile(const std::filesystem::path& path);
    /// sound를 넘겨받아 음원을 바꿉니다. 넘겨받은 소리는 멈춘 상태로 컴포넌트의 볼륨을 적용하며,
    /// 자동으로 재생하지 않습니다.
    void setSound(Sound&& sound);
    /// 음원을 지웁니다. 재생 중이던 소리도 멈춥니다.
    void clearSound();
    /// 음원을 가지고 있으면 true입니다. 그 음원이 제대로 로드되었는지는 뜻하지 않습니다.
    [[nodiscard]] bool hasSound() const noexcept;

    /// 재생합니다. 일시정지 중이면 이어서, 재생 중이면 처음부터 다시 재생합니다.
    /// 음원이 없으면 재생 함수는 아무것도 하지 않습니다.
    void play();
    /// 일시정지합니다.
    void pause();
    /// 멈추고 재생 위치를 처음으로 되돌립니다.
    void stop();
    /// 지금 재생 중이면 true입니다.
    [[nodiscard]] bool isPlaying() const;

    /// 볼륨입니다. 0~1로 자르고 NaN은 무시하며 기본값은 1입니다. 음원을 바꿔도 유지됩니다.
    /// \{
    void setVolume(float volume);
    [[nodiscard]] float volume() const noexcept;
    /// \}

private:
    std::optional<Sound> sound_;
    float volume_{1.0F};
};

} // namespace meno

#endif // MENO_AUDIO_SOURCE_HPP
