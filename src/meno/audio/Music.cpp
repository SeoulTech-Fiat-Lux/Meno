#include <meno/audio/Music.hpp>

#include "audio/Volume.hpp"

#include <SFML/Audio/Music.hpp>
#include <optional>

namespace meno {

struct Music::Impl {
    // 빈 Music의 상태 조회·설정은 오디오 장치가 없는 환경에서도 가능해야 한다.
    // 실제 SFML 스트림은 파일을 열 때만 생성한다.
    std::unique_ptr<sf::Music> music;
    float volume{1.0F};
    bool looping{false};
};

Music::Music() : impl_(std::make_unique<Impl>()) {}

Music::~Music() = default;

Music::Music(Music&&) noexcept = default;

Music& Music::operator=(Music&&) noexcept = default;

std::optional<Music> Music::openFromFile(const std::filesystem::path& path) {
    Music result;
    result.impl_->music = std::make_unique<sf::Music>();
    if (!result.impl_->music->openFromFile(path)) {
        return std::nullopt;
    }
    result.impl_->music->setVolume(audio_detail::toBackendVolume(result.impl_->volume));
    result.impl_->music->setLooping(result.impl_->looping);
    return result;
}

void Music::play() {
    if (impl_ != nullptr && impl_->music != nullptr) {
        impl_->music->play();
    }
}

void Music::pause() {
    if (impl_ != nullptr && impl_->music != nullptr) {
        impl_->music->pause();
    }
}

void Music::stop() {
    if (impl_ != nullptr && impl_->music != nullptr) {
        impl_->music->stop();
    }
}

void Music::setLooping(bool looping) {
    if (impl_ != nullptr) {
        impl_->looping = looping;
        if (impl_->music != nullptr) {
            impl_->music->setLooping(looping);
        }
    }
}

bool Music::isLooping() const {
    return impl_ != nullptr && impl_->looping;
}

void Music::setVolume(float volume) {
    // 잘못된 계산에서 전달된 NaN으로 현재 음량 상태가 오염되지 않게 한다.
    const std::optional normalized = audio_detail::normalizeVolume(volume);
    if (impl_ == nullptr || !normalized) {
        return;
    }
    impl_->volume = *normalized;
    if (impl_->music != nullptr) {
        impl_->music->setVolume(audio_detail::toBackendVolume(impl_->volume));
    }
}

float Music::volume() const {
    return impl_ == nullptr ? 0.0F : impl_->volume;
}

bool Music::isPlaying() const {
    return impl_ != nullptr && impl_->music != nullptr &&
           impl_->music->getStatus() == sf::Music::Status::Playing;
}

} // namespace meno
