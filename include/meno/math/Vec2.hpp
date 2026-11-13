#ifndef MENO_VEC2_HPP
#define MENO_VEC2_HPP

#include <cmath>
#include <cstdint>
#include <type_traits>

namespace meno {

/// 정수 벡터라도 "길이"와 "방향"은 실수로 처리해야 한다.
/// 정밀도를 유지하기 위해 정수 타입은 float로 승격, double 벡터는 double로 처리한다.
template <typename T> using RealOf = std::conditional_t<std::is_floating_point_v<T>, T, float>;

/// 2D 벡터. 렌더/입력/충돌 계층이 공용으로 쓰는 기본 좌표 타입.
template <typename T> struct Vec2 {
    T x{};
    T y{};

    constexpr Vec2() = default;
    constexpr Vec2(T inX, T inY) : x(inX), y(inY) {};

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

    [[nodiscard]] constexpr Vec2 scaledBy(const Vec2& rhs) const { return {x * rhs.x, y * rhs.y}; }

    /// 정수 벡터에서는 제곱과 합이 T의 범위를 넘으면 오버플로가 발생할 수 있다.
    [[nodiscard]] constexpr T lengthSquared() const { return x * x + y * y; }

    /// 벡터의 길이를 반환. 정수 벡터에서도 잘리지 않도록 실수 타입으로 반환.
    /// c.f. Vec2i{1, 1}.length() == 1.414...
    [[nodiscard]] RealOf<T> length() const {
        return std::sqrt(static_cast<RealOf<T>>(lengthSquared()));
    }

    /// 벡터의 단위 벡터를 반환. 길이가 0이면 {0, 0}을 반환하여 (divided by 0 미연에 방지).
    /// 정수 벡터에 쓰면 Vec2f가 반환되어, 정수 나눗셈이 야기할 수 있는 방향 왜곡 문제를 회피.
    /// c.f. Vec2i{3, 4}.normalized() == Vec2f{0.6, 0.8}
    [[nodiscard]] Vec2<RealOf<T>> normalized() const {
        using R = RealOf<T>;
        const R len = length();
        if (len == R{}) {
            return {};
        }
        return {static_cast<R>(x) / len, static_cast<R>(y) / len};
    }
};

template <typename T> constexpr Vec2<T> operator-(const Vec2<T>& v) {
    return {-v.x, -v.y};
}

template <typename T> constexpr Vec2<T> operator+(Vec2<T> lhs, const Vec2<T>& rhs) {
    return lhs += rhs;
}

template <typename T> constexpr Vec2<T> operator-(Vec2<T> lhs, const Vec2<T>& rhs) {
    return lhs -= rhs;
}

template <typename T> constexpr Vec2<T> operator*(Vec2<T> v, T scalar) {
    return v *= scalar;
}

template <typename T> constexpr Vec2<T> operator*(T scalar, Vec2<T> v) {
    return v *= scalar;
}

template <typename T> constexpr Vec2<T> operator/(Vec2<T> lhs, T scalar) {
    return lhs /= scalar;
}

template <typename T> constexpr bool operator==(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename T> constexpr T dot(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec2u = Vec2<unsigned int>;

} // namespace meno

#endif // MENO_VEC2_HPP
