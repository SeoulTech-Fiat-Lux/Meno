#include <meno/audio/Sound.hpp>
#include <meno/audio/SoundBuffer.hpp>

#include "audio/SoundBufferImpl.hpp"
#include "audio/Volume.hpp"

#include <SFML/Audio/Sound.hpp>
#include <algorithm>
#include <utility>

namespace meno {

struct Sound::Impl {
    std::shared_ptr<const SoundBuffer> buffer;
    std::unique_ptr<sf::Sound> sound;
    float volume{1.0F};
};

Sound::Sound() : impl_(std::make_unique<Impl>()) {}

Sound::Sound(std::shared_ptr<const SoundBuffer> buffer) : Sound() {
    if (buffer == nullptr || buffer->impl_ == nullptr || !buffer->impl_->loaded) {
        return;
    }

    impl_->buffer = std::move(buffer);
    impl_->sound = std::make_unique<sf::Sound>(impl_->buffer->impl_->buffer);
}

Sound::~Sound() = default;

Sound::Sound(Sound&&) noexcept = default;

Sound& Sound::operator=(Sound&&) noexcept = default;

std::optional<Sound> Sound::loadFromFile(const std::filesystem::path& path) {
    auto loaded = SoundBuffer::loadFromFile(path);
    if (!loaded) {
        return std::nullopt;
    }

    auto buffer = std::make_shared<SoundBuffer>(std::move(*loaded));
    return Sound{std::move(buffer)};
}

void Sound::play() {
    if (impl_ != nullptr && impl_->sound != nullptr) {
        impl_->sound->play();
    }
}

void Sound::pause() {
    if (impl_ != nullptr && impl_->sound != nullptr) {
        impl_->sound->pause();
    }
}

void Sound::stop() {
    if (impl_ != nullptr && impl_->sound != nullptr) {
        impl_->sound->stop();
    }
}

void Sound::setVolume(float volume) {
    // 잘못된 계산에서 전달된 NaN으로 현재 음량 상태가 오염되지 않게 한다.
    if (impl_ != nullptr && audio_detail::isValidVolume(volume)) {
        impl_->volume = std::clamp(volume, 0.0F, 1.0F);
        if (impl_->sound != nullptr) {
            impl_->sound->setVolume(audio_detail::toBackendVolume(impl_->volume));
        }
    }
}

float Sound::volume() const {
    return impl_ == nullptr ? 0.0F : impl_->volume;
}

bool Sound::isPlaying() const {
    return impl_ != nullptr && impl_->sound != nullptr &&
           impl_->sound->getStatus() == sf::Sound::Status::Playing;
}

} // namespace meno
