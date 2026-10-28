#include <meno/audio/SoundBuffer.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Texture.hpp>
#include <meno/resources/ResourceManager.hpp>

#include "check.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

static_assert(!std::is_copy_constructible_v<meno::ResourceManager>);
static_assert(std::is_nothrow_move_constructible_v<meno::ResourceManager>);
static_assert(std::is_same_v<
              decltype(std::declval<meno::ResourceManager&>().loadTexture(
                  std::declval<const std::filesystem::path&>())),
              std::shared_ptr<const meno::Texture>>);
static_assert(std::is_same_v<
              decltype(std::declval<meno::ResourceManager&>().loadFont(
                  std::declval<const std::filesystem::path&>())),
              std::shared_ptr<const meno::Font>>);
static_assert(std::is_same_v<
              decltype(std::declval<meno::ResourceManager&>().loadSoundBuffer(
                  std::declval<const std::filesystem::path&>())),
              std::shared_ptr<const meno::SoundBuffer>>);

namespace {
int textureLoads = 0;
int fontLoads = 0;
int soundBufferLoads = 0;
} // namespace

// ResourceManager의 캐시 로직만 검사하기 위한 최소 테스트 대역이다.
namespace meno {

struct Texture::Impl {};
Texture::Texture() : impl_(std::make_unique<Impl>()) {}
Texture::~Texture() = default;
Texture::Texture(Texture&&) noexcept = default;
Texture& Texture::operator=(Texture&&) noexcept = default;
std::optional<Texture> Texture::loadFromFile(const std::filesystem::path& path) {
    ++textureLoads;
    if (path.filename() == "missing.png") {
        return std::nullopt;
    }
    return Texture{};
}

struct Font::Impl {};
Font::Font() : impl_(std::make_unique<Impl>()) {}
Font::~Font() = default;
Font::Font(Font&&) noexcept = default;
Font& Font::operator=(Font&&) noexcept = default;
std::optional<Font> Font::loadFromFile(const std::filesystem::path& path) {
    ++fontLoads;
    if (path.filename() == "missing.ttf") {
        return std::nullopt;
    }
    return Font{};
}

struct SoundBuffer::Impl {};
SoundBuffer::SoundBuffer() : impl_(std::make_unique<Impl>()) {}
SoundBuffer::~SoundBuffer() = default;
SoundBuffer::SoundBuffer(SoundBuffer&&) noexcept = default;
SoundBuffer& SoundBuffer::operator=(SoundBuffer&&) noexcept = default;
std::optional<SoundBuffer> SoundBuffer::loadFromFile(const std::filesystem::path& path) {
    ++soundBufferLoads;
    if (path.filename() == "missing.wav") {
        return std::nullopt;
    }
    return SoundBuffer{};
}

} // namespace meno

int main() {
    std::shared_ptr<const meno::Texture> retainedTexture;
    {
        meno::ResourceManager resources;

        const auto texture = resources.loadTexture("assets/player.png");
        const auto sameTexture = resources.loadTexture("assets/./player.png");
        MENO_CHECK(texture != nullptr);
        MENO_CHECK(texture == sameTexture);
        MENO_CHECK(textureLoads == 1);

        const auto font = resources.loadFont("assets/main.ttf");
        const auto sameFont = resources.loadFont("assets/./main.ttf");
        MENO_CHECK(font != nullptr);
        MENO_CHECK(font == sameFont);
        MENO_CHECK(fontLoads == 1);

        const auto soundBuffer = resources.loadSoundBuffer("assets/shot.wav");
        const auto sameSoundBuffer = resources.loadSoundBuffer("assets/./shot.wav");
        MENO_CHECK(soundBuffer != nullptr);
        MENO_CHECK(soundBuffer == sameSoundBuffer);
        MENO_CHECK(soundBufferLoads == 1);

        retainedTexture = texture;

        MENO_CHECK(resources.loadTexture("missing.png") == nullptr);
        MENO_CHECK(resources.loadTexture("missing.png") == nullptr);
        MENO_CHECK(textureLoads == 3); // 실패는 캐시하지 않아 두 번 모두 로드한다.

        MENO_CHECK(resources.loadFont("missing.ttf") == nullptr);
        MENO_CHECK(resources.loadFont("missing.ttf") == nullptr);
        MENO_CHECK(fontLoads == 3);

        MENO_CHECK(resources.loadSoundBuffer("missing.wav") == nullptr);
        MENO_CHECK(resources.loadSoundBuffer("missing.wav") == nullptr);
        MENO_CHECK(soundBufferLoads == 3);
    }

    // 매니저가 먼저 소멸해도 외부 shared_ptr이 리소스 수명을 유지한다.
    MENO_CHECK(retainedTexture != nullptr);

    return meno::test::report();
}
