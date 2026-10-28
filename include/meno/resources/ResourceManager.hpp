#ifndef MENO_RESOURCE_MANAGER_HPP
#define MENO_RESOURCE_MANAGER_HPP

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace meno {

class Font;
class SoundBuffer;
class Texture;

/// 파일 경로별로 Texture, Font, SoundBuffer를 한 번만 로드하고 공유하는 캐시.
///
/// 성공하면 같은 경로에 대해 같은 shared_ptr을 반환한다. 실패하면 빈 shared_ptr을
/// 반환하며 실패 결과는 캐시하지 않으므로 다음 요청에서 다시 로드를 시도한다.
/// 반환된 리소스는 ResourceManager가 소멸해도 shared_ptr을 가진 사용자가 있는 동안
/// 유지된다. 여러 스레드에서 동시에 호출하는 동작은 지원하지 않는다.
class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) noexcept = default;
    ResourceManager& operator=(ResourceManager&&) noexcept = default;

    /// 이미지 파일을 로드한다. 같은 정규화 경로는 기존 Texture를 공유한다.
    [[nodiscard]] std::shared_ptr<const Texture>
    loadTexture(const std::filesystem::path& path);

    /// 폰트 파일을 로드한다. 같은 정규화 경로는 기존 Font를 공유한다.
    [[nodiscard]] std::shared_ptr<const Font>
    loadFont(const std::filesystem::path& path);

    /// 효과음 샘플을 로드한다. 같은 정규화 경로는 기존 SoundBuffer를 공유한다.
    [[nodiscard]] std::shared_ptr<const SoundBuffer>
    loadSoundBuffer(const std::filesystem::path& path);

private:
    [[nodiscard]] static std::string makeKey(const std::filesystem::path& path);

    std::unordered_map<std::string, std::shared_ptr<const Texture>> textures_;
    std::unordered_map<std::string, std::shared_ptr<const Font>> fonts_;
    std::unordered_map<std::string, std::shared_ptr<const SoundBuffer>> soundBuffers_;
};

} // namespace meno

#endif // MENO_RESOURCE_MANAGER_HPP
