#ifndef MENO_ANGLE_HPP
#define MENO_ANGLE_HPP

#include <cmath>
#include <numbers>

namespace meno {

/// \addtogroup math
/// \{

/// 도(degree)를 라디안으로 바꿉니다.
///
/// meno의 각도는 전부 "도(degree), 시계 방향"입니다. 라디안은 std::sin과 같은 삼각함수에 넣을
/// 때만 씁니다. 이 함수는 각도 범위를 접지 않으므로, 필요하면 먼저 wrapDegrees()로 접습니다.
[[nodiscard]] constexpr float toRadians(float degrees) noexcept {
    return degrees * (std::numbers::pi_v<float> / 180.f);
}

/// 각도를 [0, 360)으로 접습니다. NaN과 무한대는 0으로 봅니다.
[[nodiscard]] inline float wrapDegrees(float degrees) noexcept {
    if (!std::isfinite(degrees)) {
        return 0.f;
    }
    float wrapped = std::fmod(degrees, 360.f);
    if (wrapped < 0.f) {
        wrapped += 360.f;
    }
    // -1e-6 + 360처럼 반올림되어 360이 된 경우와, -0(-720 등)을 모두 0으로 맞춘다.
    return (wrapped < 360.f && wrapped != 0.f) ? wrapped : 0.f;
}

/// \}

} // namespace meno

#endif // MENO_ANGLE_HPP
