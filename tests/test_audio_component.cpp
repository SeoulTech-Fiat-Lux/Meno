#include <meno/core/Audio.hpp>
#include <meno/core/Scene.hpp>

#include "check.hpp"

#include <limits>
#include <type_traits>
#include <utility>

// 이 타깃은 실제 Sound.cpp 대신 아래 테스트 대역을 링크한다.
// 장치 없이 컴포넌트의 소유권/호출 전달/실패 처리를 검사한다.
// 실제 오디오 구현의 링크는 별도 audio 테스트가 담당한다.
namespace {
int liveSounds = 0;
int playingSounds = 0;
float appliedVolume = 1.0F;
}

namespace meno {
struct Sound::Impl {
    bool playing{false};
    float volume{1.0F};
    Impl() { ++liveSounds; }
    ~Impl() {
        --liveSounds;
        if (playing) --playingSounds;
    }
};
Sound::Sound() : impl_(std::make_unique<Impl>()) {}
Sound::~Sound() = default;
Sound::Sound(Sound&&) noexcept = default;
Sound& Sound::operator=(Sound&&) noexcept = default;
std::optional<Sound> Sound::loadFromFile(const std::filesystem::path& path) {
    if (path == "missing.wav") return std::nullopt;
    return Sound{};
}
void Sound::play() {
    if (impl_ && !impl_->playing) {
        impl_->playing = true;
        ++playingSounds;
    }
}
void Sound::pause() { stop(); }
void Sound::stop() {
    if (impl_ && impl_->playing) {
        impl_->playing = false;
        --playingSounds;
    }
}
void Sound::setVolume(float volume) {
    if (impl_) appliedVolume = impl_->volume = volume;
}
float Sound::volume() const { return impl_ ? impl_->volume : 0.0F; }
bool Sound::isPlaying() const { return impl_ && impl_->playing; }
} // namespace meno

static_assert(std::is_base_of_v<meno::Component, meno::Audio>);
static_assert(!std::is_copy_constructible_v<meno::Audio>);
static_assert(!std::is_move_constructible_v<meno::Audio>);

int main() {
    meno::Scene scene;
    auto& object = scene.createGameObject();
    const auto id = object.id();
    auto& audio = object.addComponent<meno::Audio>();
    MENO_CHECK(object.getComponent<meno::Audio>() == &audio);
    MENO_CHECK(audio.parent == &object);
    MENO_CHECK(liveSounds == 0);
    MENO_CHECK(!audio.hasSound());
    MENO_CHECK(!audio.isPlaying());
    MENO_CHECK(audio.volume() == 1.0F);
    audio.play();
    audio.pause();
    audio.stop();
    audio.clearSound();

    audio.setVolume(-1.0F);
    MENO_CHECK(audio.volume() == 0.0F);
    audio.setVolume(2.0F);
    MENO_CHECK(audio.volume() == 1.0F);
    audio.setVolume(0.25F);
    audio.setVolume(std::numeric_limits<float>::quiet_NaN());
    MENO_CHECK(audio.volume() == 0.25F);

    MENO_CHECK(audio.loadFromFile("first.wav"));
    MENO_CHECK(audio.hasSound());
    MENO_CHECK(appliedVolume == 0.25F);
    MENO_CHECK(!audio.isPlaying());
    audio.play();
    MENO_CHECK(audio.isPlaying());
    MENO_CHECK(!audio.loadFromFile("missing.wav"));
    MENO_CHECK(audio.isPlaying());
    MENO_CHECK(liveSounds == 1);
    MENO_CHECK(audio.volume() == 0.25F);
    audio.pause();
    MENO_CHECK(!audio.isPlaying());
    audio.play();
    MENO_CHECK(audio.isPlaying());
    audio.stop();
    MENO_CHECK(!audio.isPlaying());

    audio.play();
    MENO_CHECK(audio.loadFromFile("second.wav"));
    MENO_CHECK(liveSounds == 1);
    MENO_CHECK(playingSounds == 0);
    audio.setVolume(0.5F);
    MENO_CHECK(appliedVolume == 0.5F);
    audio.play();
    audio.clearSound();
    MENO_CHECK(liveSounds == 0);
    MENO_CHECK(playingSounds == 0);
    MENO_CHECK(!audio.hasSound());
    MENO_CHECK(audio.volume() == 0.5F);

    meno::Sound sound;
    sound.play();
    audio.setSound(std::move(sound));
    MENO_CHECK(!sound.isPlaying());
    MENO_CHECK(!audio.isPlaying());
    MENO_CHECK(appliedVolume == 0.5F);
    audio.play();
    auto& replacement = object.addComponent<meno::Audio>();
    MENO_CHECK(replacement.parent == &object);
    MENO_CHECK(liveSounds == 0);
    MENO_CHECK(playingSounds == 0);
    MENO_CHECK(replacement.loadFromFile("third.wav"));
    replacement.play();
    MENO_CHECK(scene.deleteGameObject(id));
    MENO_CHECK(liveSounds == 0);
    MENO_CHECK(playingSounds == 0);

    {
        meno::Audio owned{meno::Sound{}};
        owned.play();
        MENO_CHECK(playingSounds == 1);
    }
    MENO_CHECK(liveSounds == 0);
    MENO_CHECK(playingSounds == 0);
    return meno::test::report();
}
