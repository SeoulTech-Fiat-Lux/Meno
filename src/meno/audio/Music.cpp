#include <meno/audio/Music.hpp>

#include "audio/Volume.hpp"

#include <SFML/Audio/Music.hpp>

#include <utility>

namespace meno {

struct Music::Impl {
    sf::Music music;
    bool loaded{false};
};

Music::Music() : impl_(std::make_unique<Impl>()) {}

Music::~Music() = default;

Music::Music(Music&&) noexcept = default;

Music& Music::operator=(Music&&) noexcept = default;

std::optional<Music> Music::openFromFile(const std::filesystem::path& path) {
    Music result;
    if (!result.impl_->music.openFromFile(path)) {
        return std::nullopt;
    }
    result.impl_->loaded = true;
    return result;
}

void Music::play() {
    if (impl_ != nullptr && impl_->loaded) {
        impl_->music.play();
    }
}

void Music::pause() {
    if (impl_ != nullptr && impl_->loaded) {
        impl_->music.pause();
    }
}

void Music::stop() {
    if (impl_ != nullptr && impl_->loaded) {
        impl_->music.stop();
    }
}

void Music::setVolume(float volume) {
    // 잘못된 계산에서 전달된 NaN으로 현재 음량 상태가 오염되지 않게 한다.
    if (impl_ != nullptr && audio_detail::isValidVolume(volume)) {
        impl_->music.setVolume(audio_detail::toBackendVolume(volume));
    }
}

float Music::volume() const {
    return impl_ == nullptr ? 0.0F : audio_detail::fromBackendVolume(impl_->music.getVolume());
}

bool Music::isPlaying() const {
    return impl_ != nullptr && impl_->loaded && impl_->music.getStatus() == sf::Music::Status::Playing;
}

} // namespace meno
