#include <meno/resources/ResourceManager.hpp>

#include <meno/audio/SoundBuffer.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Texture.hpp>

#include <utility>

namespace meno {

std::shared_ptr<const Texture>
ResourceManager::loadTexture(const std::filesystem::path& path) {
    const std::string key = makeKey(path);
    if (const auto found = textures_.find(key); found != textures_.end()) {
        return found->second;
    }

    auto loaded = Texture::loadFromFile(path);
    if (!loaded) {
        return {};
    }

    auto resource = std::make_shared<Texture>(std::move(*loaded));
    textures_.emplace(key, resource);
    return resource;
}

std::shared_ptr<const Font>
ResourceManager::loadFont(const std::filesystem::path& path) {
    const std::string key = makeKey(path);
    if (const auto found = fonts_.find(key); found != fonts_.end()) {
        return found->second;
    }

    auto loaded = Font::loadFromFile(path);
    if (!loaded) {
        return {};
    }

    auto resource = std::make_shared<Font>(std::move(*loaded));
    fonts_.emplace(key, resource);
    return resource;
}

std::shared_ptr<const SoundBuffer>
ResourceManager::loadSoundBuffer(const std::filesystem::path& path) {
    const std::string key = makeKey(path);
    if (const auto found = soundBuffers_.find(key); found != soundBuffers_.end()) {
        return found->second;
    }

    auto loaded = SoundBuffer::loadFromFile(path);
    if (!loaded) {
        return {};
    }

    auto resource = std::make_shared<SoundBuffer>(std::move(*loaded));
    soundBuffers_.emplace(key, resource);
    return resource;
}

std::string ResourceManager::makeKey(const std::filesystem::path& path) {
    return path.lexically_normal().generic_string();
}

} // namespace meno
