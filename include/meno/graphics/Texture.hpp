#ifndef MENO_TEXTURE_HPP
#define MENO_TEXTURE_HPP

#include <meno/math/Vec2.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

/// \addtogroup graphics
/// \{

namespace backend {
struct TextureAccess;
}

// 이 헤더에는 SFML이 전혀 등장하지 않는다.
/// GPU에 올라간 이미지입니다.
///
/// 복사할 수 없고 이동할 수 있으며, 소멸하면 GPU 메모리를 돌려줍니다. 로딩 실패는 예외가 아니라
/// 빈 optional로 알립니다.
///
/// 이동된 Texture는 빈 텍스처처럼 동작합니다. size()는 {0, 0}, isSmooth()는 false이고
/// setSmooth()는 무시하며, 그려도 아무것도 나오지 않습니다. 새 Texture를 이동 대입하면 다시 쓸 수
/// 있습니다.
///
/// 같은 파일을 여러 곳에서 쓰려면 ResourceManager::loadTexture()로 공유합니다. Renderer는 그리는
/// 호출 안에서만 텍스처를 읽으므로, 텍스처는 그 호출 동안만 살아 있으면 됩니다.
///
/// \code{.cpp}
/// std::optional<meno::Texture> player = meno::Texture::loadFromFile("assets/player.png");
/// if (player) {
///     renderer.draw(*player, {100.f, 50.f});
/// }
/// \endcode
///
/// \see Renderer::draw, SpriteParams, ResourceManager::loadTexture
class Texture {
public:
    /// 빈 텍스처를 만듭니다. 이 텍스처로는 아무것도 그려지지 않습니다.
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    /// 이동합니다. 이동된 쪽은 빈 텍스처처럼 동작합니다.
    /// \{
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;
    /// \}

    /// 이미지 파일(PNG, JPG, BMP 등)을 읽어 만듭니다. 실패하면 std::nullopt를 돌려줍니다.
    [[nodiscard]] static std::optional<Texture> loadFromFile(const std::filesystem::path& path);

    /// RGBA8 픽셀 배열에서 만듭니다. 에셋 파일 없이 테스트 텍스처를 만들 때 씁니다.
    ///
    /// rgba는 왼쪽 위부터 한 줄씩, size.x × size.y × 4바이트 이상이어야 합니다. 이 호출 안에서
    /// 복사하므로 호출이 끝나면 버려도 됩니다. rgba가 nullptr이거나 size의 한 축이 0이면
    /// std::nullopt를 돌려줍니다.
    [[nodiscard]] static std::optional<Texture> fromPixels(Vec2u size, const std::uint8_t* rgba);

    /// 픽셀 크기입니다. 빈 텍스처면 {0, 0}입니다.
    [[nodiscard]] Vec2u size() const;

    /// 확대·축소할 때 선형 보간(부드럽게 섞기)을 할지 정합니다. 기본은 false입니다.
    /// 픽셀아트는 false로 둬야 뭉개지지 않습니다.
    /// \{
    void setSmooth(bool smooth);
    [[nodiscard]] bool isSmooth() const;
    /// \}

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend struct backend::TextureAccess;
};

/// \}

} // namespace meno

#endif // MENO_TEXTURE_HPP
