#include <meno/audio/Music.hpp>

#include <SFML/Audio/Music.hpp>

#include <algorithm>
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
    if (impl_ != nullptr) {
        impl_->music.setVolume(std::clamp(volume, 0.0F, 1.0F) * 100.0F);
    }
}

float Music::volume() const {
    return impl_ == nullptr ? 0.0F : impl_->music.getVolume() / 100.0F;
}

bool Music::isPlaying() const {
    return impl_ != nullptr && impl_->loaded && impl_->music.getStatus() == sf::Music::Status::Playing;
}

} // namespace meno
