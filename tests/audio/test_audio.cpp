#include <meno/audio/Music.hpp>
#include <meno/audio/Sound.hpp>
#include <meno/audio/SoundBuffer.hpp>
#include <meno/scene/AudioSource.hpp>

#include "audio/Volume.hpp"

#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "check.hpp"

static_assert(!std::is_copy_constructible_v<meno::Sound>);
static_assert(!std::is_copy_assignable_v<meno::Sound>);
static_assert(std::is_nothrow_move_constructible_v<meno::Sound>);
static_assert(std::is_nothrow_move_assignable_v<meno::Sound>);

static_assert(!std::is_copy_constructible_v<meno::SoundBuffer>);
static_assert(!std::is_copy_assignable_v<meno::SoundBuffer>);
static_assert(std::is_nothrow_move_constructible_v<meno::SoundBuffer>);
static_assert(std::is_nothrow_move_assignable_v<meno::SoundBuffer>);

static_assert(!std::is_copy_constructible_v<meno::Music>);
static_assert(!std::is_copy_assignable_v<meno::Music>);
static_assert(std::is_nothrow_move_constructible_v<meno::Music>);
static_assert(std::is_nothrow_move_assignable_v<meno::Music>);

static_assert(std::is_same_v<decltype(meno::Sound::loadFromFile(std::filesystem::path{})),
                             std::optional<meno::Sound>>);
static_assert(std::is_same_v<decltype(meno::SoundBuffer::loadFromFile(std::filesystem::path{})),
                             std::optional<meno::SoundBuffer>>);
static_assert(std::is_same_v<decltype(meno::Music::openFromFile(std::filesystem::path{})),
                             std::optional<meno::Music>>);

namespace {

// 구현 심볼을 참조해 meno와 SFML::Audio가 실제로 링크되는지 빌드 단계에서 검사한다.
using SoundLoadFunction = std::optional<meno::Sound> (*)(const std::filesystem::path&);
using BufferLoadFunction = std::optional<meno::SoundBuffer> (*)(const std::filesystem::path&);
using MusicOpenFunction = std::optional<meno::Music> (*)(const std::filesystem::path&);
using SoundCommand      = void (meno::Sound::*)();
using MusicCommand      = void (meno::Music::*)();
using MusicLoopSetter   = void (meno::Music::*)(bool);
using MusicLoopGetter   = bool (meno::Music::*)() const;

SoundLoadFunction volatile soundLoadFunction = &meno::Sound::loadFromFile;
BufferLoadFunction volatile bufferLoadFunction = &meno::SoundBuffer::loadFromFile;
MusicOpenFunction volatile musicOpenFunction = &meno::Music::openFromFile;
SoundCommand volatile      soundPlayFunction  = &meno::Sound::play;
MusicCommand volatile      musicPlayFunction  = &meno::Music::play;
MusicLoopSetter volatile   musicLoopSetter    = &meno::Music::setLooping;
MusicLoopGetter volatile   musicLoopGetter    = &meno::Music::isLooping;

} // namespace

int main() {
    meno::Sound empty{std::shared_ptr<const meno::SoundBuffer>{}};
    empty.play();
    MENO_CHECK(!empty.isPlaying());
    empty.setVolume(0.5F);
    MENO_CHECK(empty.volume() == 0.5F);

    // 빈 컴포넌트는 실제 Sound 구현과 링크하되 장치를 생성하지 않는다.
    meno::AudioSource audio;
    audio.play();
    audio.pause();
    audio.stop();
    MENO_CHECK(!audio.hasSound());
    MENO_CHECK(!audio.isPlaying());
    audio.setVolume(0.25F);
    MENO_CHECK(audio.volume() == 0.25F);

    // 오디오 객체를 만들지 않아 장치가 없는 CI에서도 실행할 수 있다.
    constexpr float nan = std::numeric_limits<float>::quiet_NaN();
    MENO_CHECK(!meno::audio_detail::isValidVolume(nan));
    MENO_CHECK(meno::audio_detail::toBackendVolume(nan) == 0.0F);

    MENO_CHECK(meno::audio_detail::toBackendVolume(-0.5F) == 0.0F);
    MENO_CHECK(meno::audio_detail::toBackendVolume(0.0F) == 0.0F);
    MENO_CHECK(meno::audio_detail::toBackendVolume(0.25F) == 25.0F);
    MENO_CHECK(meno::audio_detail::toBackendVolume(1.0F) == 100.0F);
    MENO_CHECK(meno::audio_detail::toBackendVolume(1.5F) == 100.0F);

    MENO_CHECK(meno::audio_detail::fromBackendVolume(0.0F) == 0.0F);
    MENO_CHECK(meno::audio_detail::fromBackendVolume(25.0F) == 0.25F);
    MENO_CHECK(meno::audio_detail::fromBackendVolume(100.0F) == 1.0F);

    meno::Music music;
    MENO_CHECK(!music.isLooping());
    music.setLooping(true);
    MENO_CHECK(music.isLooping());

    meno::Music movedMusic{std::move(music)};
    MENO_CHECK(movedMusic.isLooping());
    MENO_CHECK(!music.isLooping());
    movedMusic.setLooping(false);
    MENO_CHECK(!movedMusic.isLooping());

    return meno::test::report();
}
