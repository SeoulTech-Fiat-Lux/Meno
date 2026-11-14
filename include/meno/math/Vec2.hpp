#ifndef MENO_VEC2_HPP
#define MENO_VEC2_HPP

#include <cmath>
#include <type_traits>

namespace meno {

/// \addtogroup math
/// \{

/// 길이와 방향을 계산할 때 쓰는 실수 타입입니다. T가 정수 타입이면 float, 실수 타입이면 T
/// 그대로입니다.
template <typename T>
using RealOf = std::conditional_t<std::is_floating_point_v<T>, T, float>;

// 공개 헤더에 sf::Vector2를 드러내지 않으려고 둔 자체 타입이다.
// 메모리 배치가 sf::Vector2와 같아 백엔드 변환 비용은 사실상 없다.
/// 렌더·입력·충돌이 함께 쓰는 2D 벡터입니다. 보통은 별칭인 Vec2f, Vec2i, Vec2u를 씁니다.
///
/// 주의할 점:
/// - 곱하거나 나누는 스칼라는 원소와 타입이 같아야 합니다. Vec2f에 double을 곱하면 컴파일되지
///   않으므로, Application::update()가 받는 double 시간 간격은 float로 바꿔 씁니다.
/// - 원소 타입이 다른 벡터로는 명시적으로만 변환할 수 있습니다. 예: Vec2f(input.mousePosition())
///
/// \code{.cpp}
/// meno::Vec2f position{100.f, 50.f};
/// const meno::Vec2f velocity{120.f, 0.f};
/// position += velocity * static_cast<float>(fixedDeltaTime);
/// const meno::Vec2f toTarget = (target - position).normalized();
/// \endcode
///
/// \see dot, Rect
template <typename T>
struct Vec2 {
    T x{}; ///< x 성분(가로)
    T y{}; ///< y 성분(세로). meno에서는 아래로 갈수록 y가 커집니다.

    /// 영벡터 {0, 0}을 만듭니다.
    constexpr Vec2() = default;
    /// 두 성분으로 만듭니다.
    constexpr Vec2(T inX, T inY) : x(inX), y(inY) {}

    // 정수로 바꿀 때 값이 잘리는 것을 숨기지 않으려고 explicit으로 둔다.
    /// 원소 타입이 다른 벡터에서 만듭니다. 명시적으로만 변환할 수 있습니다.
    /// 실수에서 정수로 바꾸면 소수점 아래를 버립니다. 예: -1.5는 -1, 2.7은 2가 됩니다.
    template <typename U>
    constexpr explicit Vec2(const Vec2<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    /// 성분별 덧셈·뺄셈이나 스칼라 곱셈·나눗셈을 한 뒤에 자신을 돌려줍니다.
    /// 정수 벡터를 0으로 나누는 것은 정의되지 않은 동작입니다.
    /// \{
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
    /// \}

    /// 두 성분이 모두 같으면 같은 벡터입니다. 실수 벡터도 오차를 허용하지 않고 그대로 비교합니다.
    constexpr bool operator==(const Vec2&) const = default;

    /// 성분별로 곱합니다.
    [[nodiscard]] constexpr Vec2 scaledBy(const Vec2& rhs) const { return {x * rhs.x, y * rhs.y}; }

    /// 길이의 제곱입니다. 정수 벡터에서는 정수로 정확하게 계산합니다.
    ///
    /// Vec2i에서 x² + y²가 int 최댓값(2147483647)을 넘으면 오버플로(정의되지 않은 동작)가
    /// 일어납니다. 한 축만 쓰면 46341, 두 축이 같으면 32768부터 넘칩니다. length()와
    /// normalized()도 범위 제한이 같습니다.
    [[nodiscard]] constexpr T lengthSquared() const { return x * x + y * y; }

    /// 길이입니다. 정수 벡터에서도 실수로 돌려줍니다. 예: Vec2i{1, 1}.length()는 약 1.414입니다.
    [[nodiscard]] RealOf<T> length() const {
        return std::sqrt(static_cast<RealOf<T>>(lengthSquared()));
    }

    /// 같은 방향의 단위 벡터입니다. 길이가 0이면 {0, 0}을 돌려줍니다.
    /// 정수 벡터에서는 Vec2f를 돌려줍니다. 예: Vec2i{3, 4}.normalized()는 약 Vec2f{0.6, 0.8}입니다.
    [[nodiscard]] Vec2<RealOf<T>> normalized() const {
        // 정수 벡터를 실수로 바꿔 나눠야 정수 나눗셈으로 방향이 무너지지 않는다.
        using R = RealOf<T>;
        const R len = length();
        if (len == R{}) {
            return {};
        }
        return {static_cast<R>(x) / len, static_cast<R>(y) / len};
    }
};

/// 부호 반전, 성분별 덧셈·뺄셈, 스칼라 곱셈·나눗셈입니다.
/// 스칼라는 원소와 타입이 같아야 합니다. 예: Vec2f에는 0.5가 아니라 0.5f를 곱합니다.
/// \{
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
/// \}

/// 두 벡터의 내적입니다.
template <typename T>
constexpr T dot(const Vec2<T>& lhs, const Vec2<T>& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

using Vec2f = Vec2<float>;        ///< 월드 좌표, 속도, 크기 등 대부분의 값
using Vec2i = Vec2<int>;          ///< 마우스 위치와 같은 정수 픽셀 좌표
using Vec2u = Vec2<unsigned int>; ///< 창 크기처럼 음수가 없는 크기

/// \}

} // namespace meno

#endif // MENO_VEC2_HPP
