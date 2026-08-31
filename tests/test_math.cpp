// 공용 값 타입(Vec2, Rect, Color) 검사.
//
// SFML에 의존할 필요 없이 헤더만으로 성립하므로, SFML 빌드 없이 돌아간다.
// 렌더러와 충돌 판정이 함께 쓰는 타입이라, 시그니처를 바꾸면 여기서 먼저 걸린다.

#include <meno/math/Color.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include "check.hpp"

#include <cmath>
#include <type_traits>

using namespace meno;

// --- 컴파일 타임 검사 -------------------------------------------------------

// 정수 벡터라도 길이와 방향은 실수여야 한다.
static_assert(std::is_same_v<RealOf<int>, float>);
static_assert(std::is_same_v<RealOf<unsigned>, float>);
static_assert(std::is_same_v<RealOf<float>, float>);
static_assert(std::is_same_v<RealOf<double>, double>);

static_assert(std::is_same_v<decltype(Vec2i{}.length()), float>);
static_assert(std::is_same_v<decltype(Vec2u{}.length()), float>);
static_assert(std::is_same_v<decltype(Vec2<double>{}.length()), double>);

static_assert(std::is_same_v<decltype(Vec2i{}.normalized()), Vec2f>);
static_assert(std::is_same_v<decltype(Vec2<double>{}.normalized()), Vec2<double>>);

// lengthSquared는 원소 타입을 유지해야 한다(정수에서 정확).
static_assert(std::is_same_v<decltype(Vec2i{}.lengthSquared()), int>);
static_assert(Vec2i{3, 4}.lengthSquared() == 25);

// Rect / Color는 컴파일 타임에 계산 가능해야 한다.
static_assert(Rectf{0.f, 0.f, 10.f, 10.f}.contains(Vec2f{5.f, 5.f}));
static_assert(Color::fromHex(0x2E3440FF).r == 0x2E);

// --- 런타임 검사 ------------------------------------------------------------

namespace {

using meno::test::check;

bool near(float a, float b) { return std::fabs(a - b) < 1e-5f; }

} // namespace

int main() {
    // --- Vec2 ---------------------------------------------------------------
    // 정수 벡터에서 길이가 잘리면 안 된다.
    check(near(Vec2i{1, 1}.length(), 1.4142135f), "Vec2i{1,1}.length() == 1.414...");
    check(near(Vec2i{3, 4}.length(), 5.f), "Vec2i{3,4}.length() == 5");
    check(near(Vec2f{3.f, 4.f}.length(), 5.f), "Vec2f{3,4}.length() == 5");

    // 정수 나눗셈으로 방향이 사라지면 안 된다.
    const Vec2f dir = Vec2i{3, 4}.normalized();
    check(near(dir.x, 0.6f) && near(dir.y, 0.8f), "Vec2i{3,4}.normalized() == {0.6, 0.8}");

    const Vec2f dirNeg = Vec2i{-3, 4}.normalized();
    check(near(dirNeg.x, -0.6f) && near(dirNeg.y, 0.8f), "Vec2i{-3,4}.normalized() 부호 유지");

    // 0 벡터는 0으로 나누지 않는다.
    check(Vec2f{0.f, 0.f}.normalized() == Vec2f{0.f, 0.f}, "Vec2f{0,0}.normalized() == {0,0}");
    check(Vec2i{0, 0}.normalized() == Vec2f{0.f, 0.f}, "Vec2i{0,0}.normalized() == {0,0}");

    check(near(Vec2i{7, -11}.normalized().length(), 1.f), "normalized()의 길이는 1");

    check(Vec2f{1.f, 2.f} + Vec2f{3.f, 4.f} == Vec2f{4.f, 6.f}, "Vec2 덧셈");
    check(2.f * Vec2f{3.f, 4.f} == Vec2f{6.f, 8.f}, "스칼라 * 벡터 (좌측 피연산자)");
    check(near(dot(Vec2f{1.f, 2.f}, Vec2f{3.f, 4.f}), 11.f), "내적");

    // --- Rect ---------------------------------------------------------------
    const Rectf a{0.f, 0.f, 10.f, 10.f};
    const Rectf b{5.f, 5.f, 10.f, 10.f};
    const Rectf far{100.f, 100.f, 10.f, 10.f};

    check(a.intersects(b), "겹치는 사각형");
    check(!a.intersects(far), "떨어진 사각형");

    // 경계 규칙: 왼쪽/위는 포함, 오른쪽/아래는 제외.
    // 이게 지켜져야 인접 타일이 한 점을 동시에 소유하지 않는다.
    check(a.contains(Vec2f{0.f, 0.f}), "좌상단 모서리는 포함");
    check(!a.contains(Vec2f{10.f, 5.f}), "우측 경계는 제외");
    check(!a.contains(Vec2f{5.f, 10.f}), "하단 경계는 제외");

    const auto hit = a.findIntersection(b);
    check(hit.has_value(), "교집합이 존재");
    check(hit && hit->position == Vec2f{5.f, 5.f} && hit->size == Vec2f{5.f, 5.f},
          "교집합 영역이 정확");
    check(!a.findIntersection(far).has_value(), "겹치지 않으면 nullopt");

    check(a.center() == Vec2f{5.f, 5.f}, "중심점");

    // --- Color --------------------------------------------------------------
    const Color c = Color::fromHex(0x2E3440FF);
    check(c.r == 0x2E && c.g == 0x34 && c.b == 0x40 && c.a == 0xFF, "fromHex 바이트 분해");
    check(colors::White == Color{255, 255, 255, 255}, "기본값은 불투명 흰색");
    check(c.withAlpha(128).a == 128 && c.withAlpha(128).r == c.r, "withAlpha는 alpha만 바꾼다");

    return meno::test::report();
}
