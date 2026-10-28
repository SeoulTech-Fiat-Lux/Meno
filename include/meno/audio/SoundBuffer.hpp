#ifndef MENO_SOUND_BUFFER_HPP
#define MENO_SOUND_BUFFER_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

class Sound;

/// 메모리에 로드된 효과음 샘플 데이터.
///
/// 재생 상태를 가지지 않으며 여러 Sound가 shared_ptr로 안전하게 공유할 수 있다.
/// 복사할 수 없고 이동할 수 있으며, 로딩 실패는 빈 optional로 알린다.
class SoundBuffer {
public:
    SoundBuffer();
    ~SoundBuffer();

    SoundBuffer(const SoundBuffer&) = delete;
    SoundBuffer& operator=(const SoundBuffer&) = delete;
    SoundBuffer(SoundBuffer&&) noexcept;
    SoundBuffer& operator=(SoundBuffer&&) noexcept;

    [[nodiscard]] static std::optional<SoundBuffer>
    loadFromFile(const std::filesystem::path& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend class Sound;
};

} // namespace meno

#endif // MENO_SOUND_BUFFER_HPP
