#ifndef MENO_VOLUME_HPP
#define MENO_VOLUME_HPP

#include <algorithm>
#include <optional>

namespace meno::audio_detail {

/// 볼륨 규칙(Sound, Music, AudioSource 공통): 0~1로 제한하고, NaN이면 값을 버린다.
/// setVolume들은 std::nullopt를 받으면 기존 값을 유지한다.
[[nodiscard]] constexpr std::optional<float> normalizeVolume(float volume) noexcept {
    // NaN은 자기 자신과도 같지 않다. <cmath>의 std::isnan은 C++20에서
    // constexpr가 아니므로 이 비교로 대신한다.
    if (volume != volume) {
        return std::nullopt;
    }
    return std::clamp(volume, 0.0F, 1.0F);
}

/// meno의 0~1 볼륨을 SFML의 0~100으로 바꾼다. normalizeVolume을 거친 값을 넘긴다.
[[nodiscard]] constexpr float toBackendVolume(float volume) noexcept {
    return volume * 100.0F;
}

} // namespace meno::audio_detail

#endif  // MENO_VOLUME_HPP