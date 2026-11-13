#ifndef MENO_SOUND_BUFFER_HPP
#define MENO_SOUND_BUFFER_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

class Sound;

/// 메모리에 올린 효과음 데이터입니다. Sound가 이 데이터를 재생합니다.
///
/// 재생 상태가 없으므로 여러 Sound가 shared_ptr로 안전하게 공유합니다. 보통은
/// ResourceManager::loadSoundBuffer()로 얻습니다. 복사할 수 없고 이동할 수 있으며, 로딩 실패는
/// 예외가 아니라 빈 optional로 알립니다.
///
/// \see Sound, ResourceManager::loadSoundBuffer
class SoundBuffer {
public:
    /// 빈 버퍼를 만듭니다. 이 버퍼로 만든 Sound는 아무 소리도 내지 않습니다.
    SoundBuffer();
    ~SoundBuffer();

    SoundBuffer(const SoundBuffer&) = delete;
    SoundBuffer& operator=(const SoundBuffer&) = delete;
    /// 이동합니다. 이동된 쪽으로 만든 Sound는 아무 소리도 내지 않습니다.
    /// \{
    SoundBuffer(SoundBuffer&&) noexcept;
    SoundBuffer& operator=(SoundBuffer&&) noexcept;
    /// \}

    /// 오디오 파일(WAV, OGG, FLAC, MP3) 전체를 메모리에 읽습니다. 실패하면 std::nullopt를
    /// 돌려줍니다.
    [[nodiscard]] static std::optional<SoundBuffer> loadFromFile(const std::filesystem::path& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend class Sound;
};

} // namespace meno

#endif // MENO_SOUND_BUFFER_HPP
