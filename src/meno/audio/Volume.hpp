#ifndef MENO_AUDIO_VOLUME_HPP
#define MENO_AUDIO_VOLUME_HPP

#include <algorithm>

namespace meno::audio_detail {

[[nodiscard]] constexpr bool isValidVolume(float volume) noexcept {
    // NaN은 자기 자신과도 같지 않다. <cmath>의 std::isnan은 C++20에서
    // constexpr가 아니므로 이 검사는 테스트 가능한 constexpr로 유지한다.
    return volume == volume;
}

[[nodiscard]] constexpr float toBackendVolume(float volume) noexcept {
    if (!isValidVolume(volume)) {
        return 0.0F;
    }
    return std::clamp(volume, 0.0F, 1.0F) * 100.0F;
}

[[nodiscard]] constexpr float fromBackendVolume(float volume) noexcept {
    return volume / 100.0F;
}

} // namespace meno::audio_detail

#endif // MENO_AUDIO_VOLUME_HPP
