#ifndef MENO_AUDIO_SOUND_BUFFER_IMPL_HPP
#define MENO_AUDIO_SOUND_BUFFER_IMPL_HPP

#include <meno/audio/SoundBuffer.hpp>

#include <SFML/Audio/SoundBuffer.hpp>

namespace meno {

struct SoundBuffer::Impl {
    sf::SoundBuffer buffer;
    bool loaded{false};
};

} // namespace meno

#endif // MENO_AUDIO_SOUND_BUFFER_IMPL_HPP
