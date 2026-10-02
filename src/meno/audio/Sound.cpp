#include <meno/audio/Sound.hpp>

#include "audio/Volume.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

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
    // 잘못된 계산에서 전달된 NaN으로 현재 음량 상태가 오염되지 않게 한다.
    if (impl_ != nullptr && audio_detail::isValidVolume(volume)) {
        impl_->sound.setVolume(audio_detail::toBackendVolume(volume));
    }
}

float Sound::volume() const {
    return impl_ == nullptr ? 0.0F : audio_detail::fromBackendVolume(impl_->sound.getVolume());
}

bool Sound::isPlaying() const {
    return impl_ != nullptr && impl_->loaded && impl_->sound.getStatus() == sf::Sound::Status::Playing;
}

} // namespace meno
