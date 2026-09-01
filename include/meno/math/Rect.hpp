#ifndef MENO_RECT_HPP
#define MENO_RECT_HPP

#include <meno/math/Vec2.hpp>

#include <algorithm>
#include <optional>

namespace meno {

// 축 정렬 사각형.
// 렌더링뿐만 아니라, 충돌 판정에서도 이 타입을 쓰도록 하자.
// 별도의 AABB 타입을 새로 만들면 렌더와 충돌 사이에서 변환 코드가 계속 생긴다.
// 멤버 구성이 SFML 3의 sf::Rect(position/size)와 같아서 백엔드 변환도 1:1이다.
template <typename T>
struct Rect {
    // Rect가 Vec2<T> position과 Vec2<T> size를 멤버로 갖는(has-a) 구조체.
    // 상속(is-a)이 아닌 조합(has-a)관계를 선택한 이유는, Rect가 Vec2<T>의 모든 기능을 필요로 하지 않기 때문.
    Vec2<T> position{}; // 좌상단 모서리의 좌표를 나타내는 2D 벡터.
    Vec2<T> size{}; // 사각형의 너비와 높이를 나타내는 2D 벡터.

    // 생성자들. 다양한 방식으로 Rect 객체를 초기화할 수 있도록 제공.
    constexpr Rect() = default; // 기본 생성자. position과 size를 기본값으로 초기화.
    constexpr Rect(Vec2<T> inPosition, Vec2<T> inSize) : position(inPosition), size(inSize) {} // position과 size를 인자로 받는 생성자.
    constexpr Rect(T x, T y, T width, T height) : position(x, y), size(width, height) {} // 좌표와 크기를 직접 인자로 받는 생성자.

    // 사각형의 경계 좌표를 반환하는 멤버 함수들.
    [[nodiscard]] constexpr T left() const { return position.x; } // 좌측 경계
    [[nodiscard]] constexpr T top() const { return position.y; } // 상단 경계
    [[nodiscard]] constexpr T right() const { return position.x + size.x; } // 우측 경계
    [[nodiscard]] constexpr T bottom() const { return position.y + size.y; } // 하단 경계

    // 사각형의 중심 좌표를 계산하여 반환.
    [[nodiscard]] constexpr Vec2<T> center() const {
        return {position.x + size.x / T{2}, position.y + size.y / T{2}};
    }

    // 매개변수로 전달된 점이 사각형 내부에 포함되는지 여부를 판정.
    // 경계선 위의 점은 포함으로 본다(왼쪽/위는 포함, 오른쪽/아래는 제외).
    [[nodiscard]] constexpr bool contains(Vec2<T> point) const {
        return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom ();
    }

    // 두 사각형이 겹치는지 여부를 판정.
    // 제시된 네 가지 조건 중 어떤 것도 만족하지 않으면 겹치지 않는다.
    [[nodiscard]] constexpr bool intersects(const Rect& other) const {
        return left() < other.right() && other.left() < right() && top() < other.bottom() && other.top() < bottom();
    }

    // 두 사각형이 겹치는 영역을 계산하여 반환.
    // 이때 optional<Rect>를 사용하여, 겹치지 않는 경우에는 std::nullopt를 반환. (값이 있거나(결과값) 없거나(nullopt))
    [[nodiscard]] constexpr std::optional<Rect> findIntersection(const Rect& other) const {
        if (!intersects(other)) {
            return std::nullopt; // 겹치지 않으면 std::nullopt 반환.
        }
        // 겹치는 영역의 좌상단과 우하단 좌표를 계산.
        const Vec2<T> topLeft{std::max(left(), other.left()), std::max(top(), other.top())};
        const Vec2<T> bottomRight{std::min(right(), other.right()), std::min(bottom(), other.bottom())};
        // 겹치는 영역을 나타내는 Rect 객체를 반환.
        return Rect{topLeft, bottomRight - topLeft};
    }
};

using Rectf = Rect<float>; // float 타입의 Rect를 위한 별칭.
using Recti = Rect<int>; // int 타입의 Rect를 위한 별칭.

} // namespace meno

#endif  // MENO_RECT_HPP