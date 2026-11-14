#ifndef MENO_TRANSFORM_HPP
#define MENO_TRANSFORM_HPP

#include <meno/math/Angle.hpp>
#include <meno/math/Vec2.hpp>
#include <meno/scene/Component.hpp>

namespace meno {

/// \addtogroup scene
/// \{

/// GameObject의 위치, 배율, 회전입니다. 모든 GameObject가 기본으로 가집니다.
///
/// GameObject의 로컬 원점(좌상단)이 position에 놓이고, 배율과 회전은 그 점을 중심으로
/// 적용됩니다. Sprite와 Collider의 offset도 이 변환을 따릅니다.
///
/// \see GameObject::transform
struct Transform : public Component {
    /// 로컬 원점(좌상단)이 놓일 월드 위치입니다. 회전과 배율의 기준점입니다.
    Vec2f position{0, 0};
    /// 가로·세로에 함께 적용하는 배율입니다. 음수면 원점을 기준으로 뒤집히고, 0이면 크기가
    /// 없습니다.
    float scale{1.f};
    /// 회전 각도(도). 시계 방향이 양수입니다.
    float rotation{0.f};

    /// 로컬 방향 벡터에 회전만 적용합니다. 배율과 위치는 적용하지 않습니다.
    [[nodiscard]] Vec2f rotateVector(const Vec2f& vector) const {
        const float radians = toRadians(wrapDegrees(rotation));
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        return {vector.x * cosine - vector.y * sine, vector.x * sine + vector.y * cosine};
    }

    /// GameObject 좌상단 기준의 로컬 좌표를 월드 좌표로 바꿉니다. 배율, 회전, 이동 순으로
    /// 적용합니다.
    [[nodiscard]] Vec2f transformPoint(const Vec2f& point) const {
        return position + rotateVector(point * scale);
    }
};

/// \}

} // namespace meno

#endif // MENO_TRANSFORM_HPP
