#ifndef MENO_COLLIDER_HPP
#define MENO_COLLIDER_HPP

#include <meno/math/Vec2.hpp>
#include <meno/scene/Component.hpp>

namespace meno {

class GameObject;

/// 충돌 판정에 쓰는 콜라이더의 기반 구조체입니다. GameObject에는 BoxCollider나 CircleCollider를
/// 붙입니다.
///
/// 지금은 겹치는지만 판정합니다. 겹친 물체를 밀어내는 처리는 직접 합니다. 판정은 intersects()로
/// 합니다.
///
/// \code{.cpp}
/// player.addComponent<meno::BoxCollider>(meno::Vec2f{0.f, 0.f}, meno::Vec2f{32.f, 32.f});
/// coin.addComponent<meno::CircleCollider>(meno::Vec2f{8.f, 8.f}, 8.f);
/// if (meno::intersects(player, coin)) {
///     (void)scene.deleteGameObject(coin.id());
/// }
/// \endcode
///
/// \see BoxCollider, CircleCollider, intersects
struct Collider : public Component {
    /// 부모 GameObject 좌상단 기준의 로컬 위치입니다. 사각형은 좌상단, 원은 중심을 가리킵니다.
    Vec2f offset;

    /// offset이 {0, 0}인 콜라이더를 만듭니다.
    Collider() = default;
    /// offset 위치에 콜라이더를 만듭니다.
    explicit Collider(const Vec2f& offset) : offset(offset) {}
};

/// 사각형 콜라이더입니다. 부모 Transform을 따라 함께 회전하고 커집니다.
struct BoxCollider final : public Collider {
    /// 회전과 배율을 적용하기 전의 너비(x)와 높이(y)입니다. 0 이하인 축이 있으면 판정하지 않습니다.
    Vec2f size;

    /// offset과 size가 모두 {0, 0}인 콜라이더를 만듭니다. 이 상태로는 판정하지 않습니다.
    BoxCollider() = default;
    /// offset(좌상단)과 size로 만듭니다.
    BoxCollider(const Vec2f& offset, const Vec2f& size) : Collider(offset), size(size) {}

    /// 월드 좌표의 점이 사각형 안에 있는지 확인합니다.
    ///
    /// 회전된 사각형의 축을 기준으로 왼쪽·위 경계는 포함하고 오른쪽·아래 경계는 제외합니다. 부모가
    /// 없거나, size에 0 이하인 축이 있거나, 부모의 배율이 0이거나 유한하지 않으면 false입니다.
    [[nodiscard]] bool contains(Vec2f worldPoint) const;
};

/// 원형 콜라이더입니다. 부모 Transform의 배율(절댓값)만큼 반지름이 커집니다.
struct CircleCollider final : public Collider {
    /// 배율을 적용하기 전의 반지름입니다. 0 이하면 판정하지 않습니다.
    float radius{};

    /// offset이 {0, 0}이고 반지름이 0인 콜라이더를 만듭니다. 이 상태로는 판정하지 않습니다.
    CircleCollider() = default;
    /// offset(중심)과 radius로 만듭니다.
    CircleCollider(const Vec2f& offset, float radius) : Collider(offset), radius(radius) {}
};

/// 두 GameObject의 콜라이더가 겹치면 true입니다.
///
/// 각 GameObject의 BoxCollider와 CircleCollider를 모든 조합으로 검사해 하나라도 겹치면 true입니다.
/// Collider를 상속한 다른 타입은 보지 않습니다. 경계만 맞닿은 경우는 겹치지 않는 것으로 봅니다.
/// 크기나 반지름이 0 이하이거나, 부모의 배율이 0이거나 유한하지 않은 콜라이더는 겹치지 않습니다.
bool intersects(const GameObject& a, const GameObject& b);

} // namespace meno
#endif // MENO_COLLIDER_HPP
