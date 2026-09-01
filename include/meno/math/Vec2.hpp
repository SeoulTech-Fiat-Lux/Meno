#ifndef MENO_VEC2_HPP
#define MENO_VEC2_HPP

#include <cmath>
#include <cstdint>
#include <type_traits>

namespace meno {

// 정수 벡터라도 "길이"와 "방향"은 실수로 처리해야 한다.
// 정밀도를 유지하기 위해 정수 타입은 float로 승격, double 벡터는 double로 처리한다.
template <typename T>
using RealOf = std::conditional_t<std::is_floating_point_v<T>, T, float>;

// 2D 벡터. 렌더/입력/충돌 계층이 공용으로 쓰는 기본 좌표 타입.
// SFML의 sf::Vector2를 공개 헤더에 노출하지 않기 위한 자체 타입이며, 메모리 레이아웃이 동일해서 백엔드 변환 비용은 사실상 없다.
template <typename T>
struct Vec2 {
    T x{};
    T y{};

    // 가능하면 컴파일할 때 미리 계산하여, 결과물을 컴파일 시간에 상수로 반영.
    constexpr Vec2() = default;
    constexpr Vec2(T inX, T inY) : x(inX), y(inY) {};

    // 컴파일러가 임의로 Vec2<U>을 Vec2<T>로 변환하지 않도록 방지 (explicit constructor)
    // unsigned int 같은 정수 타입에서 계산 결과가 잘리는 것을 방지하기 위함.
    template <typename U>
    constexpr explicit Vec2(const Vec2<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    // 연산자 오버로딩. +=, -=, *=, /= 연산자를 제공하여 벡터 연산을 편리하게 수행할 수 있도록 함.
    // 맴버 함수로 정의되어 있어, Vec2 객체 간의 연산을 수행할 때 활용.
    constexpr Vec2& operator+=(const Vec2& rhs) { x += rhs.x; y += rhs.y; return *this; }
    constexpr Vec2& operator-=(const Vec2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    constexpr Vec2& operator*=(T scalar) { x *= scalar; y *= scalar; return *this; }
    constexpr Vec2& operator/=(T scalar) { x /= scalar; y /= scalar; return *this; }

    // 벡터의 각 성분을 다른 벡터의 대응 성분과 곱하여 새로운 스케일 벡터를 생성.
    // 객체를 바꾸지 않고 새로운 Vec2 객체를 반환하여, 원본 벡터는 그대로 유지. (const 사용 이유)
    [[nodiscard]] constexpr Vec2 scaledBy(const Vec2& rhs) const { return {x * rhs.x, y * rhs.y}; }

    // 벡터의 길이를 제곱한 값을 반환. sqrt()를 활용하지 않고도 정수 벡터에서도 정확하게 계산 가능.
    // 단, 크기가 큰 정수 벡터에서는 오버플로 발생 가능. Vec2i에서 약 46341을 초과할 때, int 범위를 벗어남.
    [[nodiscard]] constexpr T lengthSquared() const { return x * x + y * y; }

    // 벡터의 길이를 반환. 정수 벡터에서도 잘리지 않도록 실수 타입으로 반환.
    // c.f. Vec2i{1, 1}.length() == 1.414...
    [[nodiscard]] RealOf<T> length() const {
        return std::sqrt(static_cast<RealOf<T>>(lengthSquared()));
    }

    // 벡터의 단위 벡터를 반환. 길이가 0이면 {0, 0}을 반환하여 (divided by 0 미연에 방지).
    // 정수 벡터에 쓰면 Vec2f가 반환되어, 정수 나눗셈이 야기할 수 있는 방향 왜곡 문제를 회피.
    // c.f. Vec2i{3, 4}.normalized() == Vec2f{0.6, 0.8}
    [[nodiscard]] Vec2<RealOf<T>> normalized() const {
        using R = RealOf<T>;    // 정수 벡터의 경우, 길이를 실수(ReaOf<int>: int -> float)로 계산하여 방향을 유지.
        const R len = length();
        if (len == R{}) {
            return {};
        }
        return {static_cast<R>(x) / len, static_cast<R>(y) / len};
    }

};

// 전역 연산자 오버로딩. 벡터의 방향 전환, 벡터 간 덧셈, 뺄셈, 스칼라 곱셈, 스칼라 나눗셈을 지원.
template <typename T>
constexpr Vec2<T> operator-(const Vec2<T>& v) { return {-v.x, -v.y}; }

template <typename T>
constexpr Vec2<T> operator+(Vec2<T> lhs, const Vec2<T>& rhs) { return lhs += rhs; }

template <typename T>
constexpr Vec2<T> operator-(Vec2<T> lhs, const Vec2<T>& rhs) { return lhs -= rhs; }

template <typename T>
constexpr Vec2<T> operator*(Vec2<T> v, T scalar) { return v *= scalar; }

template <typename T>
constexpr Vec2<T> operator*(T scalar, Vec2<T> v) { return v *= scalar; }

template <typename T>
constexpr Vec2<T> operator/(Vec2<T> lhs, T scalar) { return lhs /= scalar; }

// +a. 벡터의 동등성 비교 연산자. 두 벡터가 같은 좌표를 가지는지 확인.
template <typename T>
constexpr bool operator==(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

// +a. 벡터의 내적 연산. 두 벡터의 방향과 크기를 고려하여 스칼라 값을 반환.
template <typename T>
constexpr T dot(const Vec2<T>& lhs, const Vec2<T>& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }

using Vec2f = Vec2<float>;  // 2D 벡터의 float 타입 정의. 렌더링, 물리 계산 등에서 사용.
using Vec2i = Vec2<int>;    // 2D 벡터의 int 타입 정의. 텍스처 픽셀 좌표에 사용.
using Vec2u = Vec2<unsigned int>;   // 2D 벡터의 unsigned int 타입 정의. 음수가 없는 크기 값에 사용.

} // namespace meno

#endif  // MENO_VEC2_HPP