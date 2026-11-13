#include <meno/resources/ResourceManager.hpp>

#include <meno/audio/SoundBuffer.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Texture.hpp>

#include <utility>

namespace meno {
namespace {

struct NoOp {
    template <typename Resource> void operator()(Resource&) const noexcept {}
};

template <typename Resource, typename Cache, typename Configure = NoOp>
std::shared_ptr<const Resource> loadResource(Cache& cache, const typename Cache::key_type& key,
                                             const std::filesystem::path& path,
                                             Configure configure = {}) {
    if (const auto found = cache.find(key); found != cache.end()) {
        return found->second;
    }

    auto loaded = Resource::loadFromFile(path);
    if (!loaded) {
        return {};
    }

    configure(*loaded);
    auto resource = std::make_shared<Resource>(std::move(*loaded));
    cache.emplace(key, resource);
    return resource;
}

} // namespace

std::shared_ptr<const Texture> ResourceManager::loadTexture(const std::filesystem::path& path,
                                                            TextureOptions options) {
    auto key = makeKey(path);
    // 파일 경로에는 NUL 문자가 올 수 없으므로 옵션 구분자로 안전하게 사용할 수 있다.
    key.push_back(u8'\0');
    key.push_back(options.smooth ? u8'1' : u8'0');
    return loadResource<Texture>(
        textures_, key, path, [options](Texture& texture) { texture.setSmooth(options.smooth); });
}

std::shared_ptr<const Font> ResourceManager::loadFont(const std::filesystem::path& path) {
    return loadResource<Font>(fonts_, makeKey(path), path);
}

std::shared_ptr<const SoundBuffer>
ResourceManager::loadSoundBuffer(const std::filesystem::path& path) {
    return loadResource<SoundBuffer>(soundBuffers_, makeKey(path), path);
}

std::u8string ResourceManager::makeKey(const std::filesystem::path& path) {
    return path.lexically_normal().generic_u8string();
}

} // namespace meno
