#include <meno/scene/Audio.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace meno {

Audio::Audio(Sound&& sound) {
    setSound(std::move(sound));
}

bool Audio::loadFromFile(const std::filesystem::path& path) {
    auto sound = Sound::loadFromFile(path);
    if (!sound) {
        return false;
    }
    setSound(std::move(*sound));
    return true;
}

void Audio::setSound(Sound&& sound) {
    sound.stop();
    sound.setVolume(volume_);
    sound_.emplace(std::move(sound));
}

void Audio::clearSound() {
    sound_.reset();
}

bool Audio::hasSound() const noexcept {
    return sound_.has_value();
}

void Audio::play() {
    if (sound_) sound_->play();
}

void Audio::pause() {
    if (sound_) sound_->pause();
}

void Audio::stop() {
    if (sound_) sound_->stop();
}

bool Audio::isPlaying() const {
    return sound_ && sound_->isPlaying();
}

void Audio::setVolume(float volume) {
    if (std::isnan(volume)) return;
    volume_ = std::clamp(volume, 0.0F, 1.0F);
    if (sound_) sound_->setVolume(volume_);
}

float Audio::volume() const noexcept {
    return volume_;
}

} // namespace meno
