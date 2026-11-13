#ifndef MENO_RECT_HPP
#define MENO_RECT_HPP

#include <meno/math/Vec2.hpp>

#include <algorithm>
#include <optional>

namespace meno {

// AABB 타입을 따로 만들면 렌더와 충돌 판정 사이에 변환 코드가 계속 생기므로 하나로 둔다.
// 멤버 구성이 SFML 3의 sf::Rect(position/size)와 같아서 백엔드 변환도 1:1이다.
/// 축 정렬 사각형(AABB)입니다. 렌더와 충돌 판정이 함께 씁니다.
///
/// 경계: contains()는 오른쪽·아래 경계를 제외하고, intersects()는 경계만 맞닿으면 겹치지 않는
/// 것으로 봅니다. size가 음수인 경우는 따로 처리하지 않습니다.
///
/// \code{.cpp}
/// const meno::Rectf player{{100.f, 50.f}, {32.f, 32.f}};
/// const meno::Rectf wall{120.f, 0.f, 16.f, 200.f};
/// if (player.intersects(wall)) {
///     const meno::Rectf overlap = *player.findIntersection(wall); // 겹친 영역
/// }
/// \endcode
///
/// \see Vec2
template <typename T>
struct Rect {
    Vec2<T> position{}; ///< 좌상단 꼭짓점
    Vec2<T> size{};     ///< 너비(x)와 높이(y)

    /// 위치와 크기가 모두 0인 사각형을 만듭니다.
    constexpr Rect() = default;
    /// 좌상단 위치와 크기로 만듭니다.
    /// \{
    constexpr Rect(Vec2<T> inPosition, Vec2<T> inSize) : position(inPosition), size(inSize) {}
    constexpr Rect(T x, T y, T width, T height) : position(x, y), size(width, height) {}
    /// \}

    /// 네 변의 좌표입니다. right()는 position.x + size.x, bottom()은 position.y + size.y입니다.
    /// \{
    [[nodiscard]] constexpr T left() const { return position.x; }
    [[nodiscard]] constexpr T top() const { return position.y; }
    [[nodiscard]] constexpr T right() const { return position.x + size.x; }
    [[nodiscard]] constexpr T bottom() const { return position.y + size.y; }
    /// \}

    /// 중심 좌표입니다. 정수 사각형은 크기를 2로 나눈 몫을 씁니다.
    /// 예: Recti{0, 0, 5, 5}.center()는 {2, 2}입니다.
    [[nodiscard]] constexpr Vec2<T> center() const {
        return {position.x + size.x / T{2}, position.y + size.y / T{2}};
    }

    /// 점이 사각형 안에 있는지 확인합니다.
    ///
    /// 왼쪽·위 경계는 포함하고 오른쪽·아래 경계는 제외하므로, 맞닿은 두 사각형에 동시에 들어가는
    /// 점은 없습니다.
    [[nodiscard]] constexpr bool contains(Vec2<T> point) const {
        return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom();
    }

    /// 두 사각형이 겹치는지 확인합니다. 경계만 맞닿은 경우는 겹치지 않는 것으로 봅니다.
    [[nodiscard]] constexpr bool intersects(const Rect& other) const {
        return left() < other.right() && other.left() < right() && top() < other.bottom() &&
               other.top() < bottom();
    }

    /// 겹치는 영역입니다. 겹치지 않으면 std::nullopt를 돌려줍니다.
    [[nodiscard]] constexpr std::optional<Rect> findIntersection(const Rect& other) const {
        if (!intersects(other)) {
            return std::nullopt;
        }
        const Vec2<T> topLeft{std::max(left(), other.left()), std::max(top(), other.top())};
        const Vec2<T> bottomRight{std::min(right(), other.right()),
                                  std::min(bottom(), other.bottom())};
        return Rect{topLeft, bottomRight - topLeft};
    }
};

using Rectf = Rect<float>; ///< 월드 좌표의 영역 대부분
using Recti = Rect<int>;   ///< 텍스처의 일부를 고르는 정수 픽셀 영역

} // namespace meno

#endif // MENO_RECT_HPP
