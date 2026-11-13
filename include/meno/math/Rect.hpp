#ifndef MENO_RECT_HPP
#define MENO_RECT_HPP

#include <meno/math/Vec2.hpp>

#include <algorithm>
#include <optional>

namespace meno {

/// 축 정렬 사각형. 렌더와 충돌 판정이 함께 쓴다.
/// AABB 타입을 따로 만들면 둘 사이에 변환 코드가 계속 생긴다.
/// 멤버 구성이 SFML 3의 sf::Rect(position/size)와 같아서 백엔드 변환도 1:1이다.
template <typename T>
struct Rect {
    Vec2<T> position{}; ///< 좌상단
    Vec2<T> size{};

    constexpr Rect() = default;
    constexpr Rect(Vec2<T> inPosition, Vec2<T> inSize) : position(inPosition), size(inSize) {}
    constexpr Rect(T x, T y, T width, T height) : position(x, y), size(width, height) {}

    [[nodiscard]] constexpr T left() const { return position.x; }
    [[nodiscard]] constexpr T top() const { return position.y; }
    [[nodiscard]] constexpr T right() const { return position.x + size.x; }
    [[nodiscard]] constexpr T bottom() const { return position.y + size.y; }

    [[nodiscard]] constexpr Vec2<T> center() const {
        return {position.x + size.x / T{2}, position.y + size.y / T{2}};
    }

    /// 왼쪽·위 경계는 포함하고 오른쪽·아래 경계는 제외한다.
    /// 맞닿은 두 사각형이 경계의 한 점을 동시에 갖지 않게 하기 위해서다.
    [[nodiscard]] constexpr bool contains(Vec2<T> point) const {
        return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom();
    }

    /// 경계만 맞닿은 경우는 겹치지 않는 것으로 본다.
    [[nodiscard]] constexpr bool intersects(const Rect& other) const {
        return left() < other.right() && other.left() < right() && top() < other.bottom() &&
               other.top() < bottom();
    }

    /// 겹치는 영역. 겹치지 않으면 std::nullopt.
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

using Rectf = Rect<float>;
using Recti = Rect<int>;

} // namespace meno

#endif // MENO_RECT_HPP
