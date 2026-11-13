#ifndef MENO_VEC2_HPP
#define MENO_VEC2_HPP

#include <cmath>
#include <type_traits>

namespace meno {

/// 정수 벡터라도 길이와 방향은 실수로 계산한다. 정수 타입은 float로, 실수 타입은 그대로 쓴다.
template <typename T>
using RealOf = std::conditional_t<std::is_floating_point_v<T>, T, float>;

/// 렌더·입력·충돌이 함께 쓰는 2D 벡터.
/// 공개 헤더에 sf::Vector2를 드러내지 않으려고 둔 자체 타입이며, 메모리 배치가 같아
/// 백엔드 변환 비용은 사실상 없다.
template <typename T>
struct Vec2 {
    T x{};
    T y{};

    constexpr Vec2() = default;
    constexpr Vec2(T inX, T inY) : x(inX), y(inY) {}

    /// 다른 원소 타입에서는 명시적으로만 바꾼다.
    /// 정수 타입으로 바꿀 때 값이 잘리는 것을 숨기지 않기 위해서다.
    template <typename U>
    constexpr explicit Vec2(const Vec2<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    constexpr Vec2& operator+=(const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    constexpr Vec2& operator-=(const Vec2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    constexpr Vec2& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }
    constexpr Vec2& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    constexpr bool operator==(const Vec2&) const = default;

    /// 성분끼리 곱한다.
    [[nodiscard]] constexpr Vec2 scaledBy(const Vec2& rhs) const { return {x * rhs.x, y * rhs.y}; }

    /// 원소 타입을 유지하므로 정수 벡터에서도 정확하다. 단 Vec2i는 성분이 약 46341을 넘으면 넘친다.
    [[nodiscard]] constexpr T lengthSquared() const { return x * x + y * y; }

    /// 정수 벡터도 잘리지 않도록 실수로 돌려준다. 예: Vec2i{1, 1}.length() == 1.414...
    [[nodiscard]] RealOf<T> length() const {
        return std::sqrt(static_cast<RealOf<T>>(lengthSquared()));
    }

    /// 단위 벡터. 길이가 0이면 {0, 0}을 돌려준다.
    /// 정수 벡터는 Vec2f로 돌려줘 정수 나눗셈으로 방향이 무너지지 않게 한다.
    /// 예: Vec2i{3, 4}.normalized() == Vec2f{0.6, 0.8}
    [[nodiscard]] Vec2<RealOf<T>> normalized() const {
        using R = RealOf<T>;
        const R len = length();
        if (len == R{}) {
            return {};
        }
        return {static_cast<R>(x) / len, static_cast<R>(y) / len};
    }
};

template <typename T>
constexpr Vec2<T> operator-(const Vec2<T>& v) {
    return {-v.x, -v.y};
}

template <typename T>
constexpr Vec2<T> operator+(Vec2<T> lhs, const Vec2<T>& rhs) {
    return lhs += rhs;
}

template <typename T>
constexpr Vec2<T> operator-(Vec2<T> lhs, const Vec2<T>& rhs) {
    return lhs -= rhs;
}

template <typename T>
constexpr Vec2<T> operator*(Vec2<T> v, T scalar) {
    return v *= scalar;
}

template <typename T>
constexpr Vec2<T> operator*(T scalar, Vec2<T> v) {
    return v *= scalar;
}

template <typename T>
constexpr Vec2<T> operator/(Vec2<T> lhs, T scalar) {
    return lhs /= scalar;
}

/// 내적.
template <typename T>
constexpr T dot(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;          ///< 텍스처 픽셀 좌표 등
using Vec2u = Vec2<unsigned int>; ///< 창 크기처럼 음수가 없는 크기

} // namespace meno

#endif // MENO_VEC2_HPP
