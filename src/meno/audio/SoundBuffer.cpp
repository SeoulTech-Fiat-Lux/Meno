#include <meno/audio/SoundBuffer.hpp>

#include "audio/SoundBufferImpl.hpp"

namespace meno {

SoundBuffer::SoundBuffer() : impl_(std::make_unique<Impl>()) {}

SoundBuffer::~SoundBuffer() = default;

SoundBuffer::SoundBuffer(SoundBuffer&&) noexcept = default;

SoundBuffer& SoundBuffer::operator=(SoundBuffer&&) noexcept = default;

std::optional<SoundBuffer>
SoundBuffer::loadFromFile(const std::filesystem::path& path) {
    SoundBuffer result;
    if (!result.impl_->buffer.loadFromFile(path)) {
        return std::nullopt;
    }
    result.impl_->loaded = true;
    return result;
}

} // namespace meno
