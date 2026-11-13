#ifndef MENO_TRANSFORM_HPP
#define MENO_TRANSFORM_HPP

#include <meno/math/Angle.hpp>
#include <meno/math/Vec2.hpp>
#include <meno/scene/Component.hpp>

namespace meno {

struct Transform : public Component {
    /// 로컬 좌상단 원점의 월드 위치 (회전/배율의 기준점).
    Vec2f position{0, 0};
    /// 균일 배율. 음수는 원점 기준 반전, 0은 크기 없음.
    float scale{1.f};
    /// 도 단위 회전.
    float rotation{0.f};

    /// 로컬 방향을 회전한다. 배율과 위치는 적용하지 않는다.
    [[nodiscard]] Vec2f rotateVector(const Vec2f& vector) const {
        const float radians = toRadians(wrapDegrees(rotation));
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        return {vector.x * cosine - vector.y * sine, vector.x * sine + vector.y * cosine};
    }

    /// 부모 좌상단 기준 로컬 좌표를 배율 → 회전 → 평행이동 순으로 변환한다.
    [[nodiscard]] Vec2f transformPoint(const Vec2f& point) const {
        return position + rotateVector(point * scale);
    }
};

} // namespace meno

#endif // MENO_TRANSFORM_HPP
