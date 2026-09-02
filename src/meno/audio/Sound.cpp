#include <meno/audio/Sound.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <algorithm>
#include <utility>

namespace meno {

struct Sound::Impl {
    sf::SoundBuffer buffer;
    sf::Sound sound{buffer};
    bool loaded{false};
};

Sound::Sound() : impl_(std::make_unique<Impl>()) {}

Sound::~Sound() = default;

Sound::Sound(Sound&&) noexcept = default;

Sound& Sound::operator=(Sound&&) noexcept = default;

std::optional<Sound> Sound::loadFromFile(const std::filesystem::path& path) {
    Sound result;
    if (!result.impl_->buffer.loadFromFile(path)) {
        return std::nullopt;
    }
    result.impl_->loaded = true;
    return result;
}

void Sound::play() {
    if (impl_ != nullptr && impl_->loaded) {
        impl_->sound.play();
    }
}

void Sound::pause() {
    if (impl_ != nullptr && impl_->loaded) {
        impl_->sound.pause();
    }
}

void Sound::stop() {
    if (impl_ != nullptr && impl_->loaded) {
        impl_->sound.stop();
    }
}

void Sound::setVolume(float volume) {
    if (impl_ != nullptr) {
        impl_->sound.setVolume(std::clamp(volume, 0.0F, 1.0F) * 100.0F);
    }
}

float Sound::volume() const {
    return impl_ == nullptr ? 0.0F : impl_->sound.getVolume() / 100.0F;
}

bool Sound::isPlaying() const {
    return impl_ != nullptr && impl_->loaded && impl_->sound.getStatus() == sf::Sound::Status::Playing;
}

} // namespace meno
