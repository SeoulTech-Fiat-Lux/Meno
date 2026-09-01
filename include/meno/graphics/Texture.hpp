#ifndef MENO_TEXTURE_HPP
#define MENO_TEXTURE_HPP

#include <meno/math/Vec2.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

namespace backend {
struct TextureAccess;
}

/// GPU에 올라간 이미지. 이 헤더에는 SFML이 전혀 등장하지 않는다.
///
/// 소유권: 복사 불가, 이동 가능. 소멸자에서 GPU 리소스를 자동 해제한다(RAII).
/// 로딩 실패는 예외가 아니라 빈 optional로 알린다.
///
/// 담당 경계: 파일 캐싱과 재사용은 ResourceManager의 몫이다. Renderer는
/// const Texture&를 받아 그리기만 하고, 수명에는 관여하지 않는다.
class Texture {
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    /// 파일에서 로드. 실패하면 std::nullopt.
    /// PNG, JPG, BMP 등 SFML이 지원하는 포맷을 따른다.
    [[nodiscard]] static std::optional<Texture> loadFromFile(const std::filesystem::path& path);

    /// RGBA8 픽셀 배열에서 만든다. 크기는 size.x * size.y * 4 바이트여야 한다.
    /// 에셋 파일 없이 테스트 텍스처를 만들 때 쓴다.
    [[nodiscard]] static std::optional<Texture> fromPixels(Vec2u size, const std::uint8_t* rgba);

    [[nodiscard]] Vec2u size() const;

    /// 확대 시 선형 보간 여부. 픽셀아트라면 false로 둬야 뭉개지지 않는다.
    void setSmooth(bool smooth);
    [[nodiscard]] bool isSmooth() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend struct backend::TextureAccess;
};

} // namespace meno

#endif  // MENO_TEXTURE_HPP