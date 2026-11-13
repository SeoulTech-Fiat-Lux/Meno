#include <meno/scene/AudioSource.hpp>

#include "audio/Volume.hpp"

#include <utility>

namespace meno {

AudioSource::AudioSource(Sound&& sound) { setSound(std::move(sound)); }

bool AudioSource::loadFromFile(const std::filesystem::path& path) {
    auto sound = Sound::loadFromFile(path);
    if (!sound) {
        return false;
    }
    setSound(std::move(*sound));
    return true;
}

void AudioSource::setSound(Sound&& sound) {
    sound.stop();
    sound.setVolume(volume_);
    sound_.emplace(std::move(sound));
}

void AudioSource::clearSound() { sound_.reset(); }

bool AudioSource::hasSound() const noexcept { return sound_.has_value(); }

void AudioSource::play() {
    if (sound_)
        sound_->play();
}

void AudioSource::pause() {
    if (sound_)
        sound_->pause();
}

void AudioSource::stop() {
    if (sound_)
        sound_->stop();
}

bool AudioSource::isPlaying() const { return sound_ && sound_->isPlaying(); }

void AudioSource::setVolume(float volume) {
    const std::optional normalized = audio_detail::normalizeVolume(volume);
    if (!normalized)
        return;
    volume_ = *normalized;
    if (sound_)
        sound_->setVolume(volume_);
}

float AudioSource::volume() const noexcept { return volume_; }

} // namespace meno
