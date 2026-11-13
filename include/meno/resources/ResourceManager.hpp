#ifndef MENO_RESOURCE_MANAGER_HPP
#define MENO_RESOURCE_MANAGER_HPP

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace meno {

/// \addtogroup resources
/// \{

class Font;
class SoundBuffer;
class Texture;

/// ResourceManager::loadTexture()의 옵션입니다.
struct TextureOptions {
    /// 확대·축소할 때 선형 보간을 할지 정합니다. Texture::setSmooth()와 같습니다.
    bool smooth{false};
};

/// 파일 경로마다 Texture, Font, SoundBuffer를 한 번만 로드해 공유하는 캐시입니다.
///
/// 같은 경로를 다시 요청하면 처음 로드한 것과 같은 shared_ptr을 돌려줍니다. 돌려준 리소스는
/// ResourceManager가 소멸해도 shared_ptr을 가진 쪽이 있는 동안 유지됩니다. Game을 쓰면
/// Game::resources()로 얻습니다.
///
/// 주의할 점:
/// - 로드에 실패하면 빈 shared_ptr을 돌려주므로 쓰기 전에 확인합니다. 실패는 캐시하지 않아서
///   다음 요청에서 다시 시도합니다.
/// - 경로는 글자 단위로만 정리합니다(./와 ../ 정리, 구분자 통일). 같은 파일이라도 상대 경로와
///   절대 경로, 대소문자가 다른 경로는 따로 로드합니다. 상대 경로는 현재 작업 디렉터리 기준입니다.
/// - 대소문자는 실제 파일명과 똑같이 씁니다. Windows에서는 틀려도 열리지만, Linux에서는 로드에
///   실패합니다.
/// - 한 번 로드한 리소스는 ResourceManager가 소멸할 때까지 캐시에 남습니다.
/// - 여러 스레드에서 동시에 부르면 안 됩니다.
///
/// \code{.cpp}
/// const auto player = game.resources().loadTexture("assets/player.png");
/// if (player) {
///     renderer.draw(*player, {100.f, 50.f});
/// }
/// \endcode
///
/// \see Texture, Font, SoundBuffer, Game::resources
class ResourceManager {
public:
    /// 빈 캐시를 만듭니다.
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    /// 이동합니다. 캐시도 함께 옮겨 갑니다.
    /// \{
    ResourceManager(ResourceManager&&) noexcept = default;
    ResourceManager& operator=(ResourceManager&&) noexcept = default;
    /// \}

    /// 이미지 파일을 로드하거나, 이미 로드했으면 캐시된 Texture를 돌려줍니다. 실패하면 빈
    /// shared_ptr입니다.
    ///
    /// 같은 경로라도 options가 다르면 따로 로드합니다. 돌려받은 Texture는 const라 setSmooth()를
    /// 부를 수 없으므로, 보간 여부는 options로 정합니다.
    [[nodiscard]] std::shared_ptr<const Texture> loadTexture(const std::filesystem::path& path,
                                                             TextureOptions options = {});

    /// 폰트 파일을 열거나, 이미 열었으면 캐시된 Font를 돌려줍니다. 실패하면 빈 shared_ptr입니다.
    ///
    /// 캐시가 Font를 붙잡고 있으므로, ResourceManager가 살아 있는 동안 폰트 파일은 열린 채로
    /// 있습니다.
    [[nodiscard]] std::shared_ptr<const Font> loadFont(const std::filesystem::path& path);

    /// 효과음 파일을 읽거나, 이미 읽었으면 캐시된 SoundBuffer를 돌려줍니다. 실패하면 빈
    /// shared_ptr입니다. 받은 버퍼는 Sound에 넘겨 재생합니다.
    [[nodiscard]] std::shared_ptr<const SoundBuffer>
    loadSoundBuffer(const std::filesystem::path& path);

private:
    [[nodiscard]] static std::u8string makeKey(const std::filesystem::path& path);

    std::unordered_map<std::u8string, std::shared_ptr<const Texture>> textures_;
    std::unordered_map<std::u8string, std::shared_ptr<const Font>> fonts_;
    std::unordered_map<std::u8string, std::shared_ptr<const SoundBuffer>> soundBuffers_;
};

/// \}

} // namespace meno

#endif // MENO_RESOURCE_MANAGER_HPP
