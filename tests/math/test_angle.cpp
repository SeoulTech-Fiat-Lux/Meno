#include <meno/math/Angle.hpp>

#include "check.hpp"

#include <cmath>
#include <limits>
#include <numbers>

using namespace meno;

// toRadians는 상수식에서도 쓸 수 있어야 한다.
static_assert(toRadians(0.f) == 0.f);

namespace {

using meno::test::check;

bool near(float a, float b) { return std::fabs(a - b) < 1e-6f; }

} // namespace

int main() {
    // --- toRadians ---------------------------------------------------------
    check(near(toRadians(180.f), std::numbers::pi_v<float>), "180도 == π");
    check(near(toRadians(-90.f), -std::numbers::pi_v<float> / 2.f), "음수 각도도 그대로 변환한다");

    // --- wrapDegrees -------------------------------------------------------
    // 아래 값들은 float로 정확히 표현되므로 ==로 비교한다.
    check(wrapDegrees(0.f) == 0.f, "0은 그대로");
    check(wrapDegrees(360.f) == 0.f, "360은 0");
    check(wrapDegrees(361.5f) == 1.5f, "한 바퀴를 넘으면 나머지");
    check(wrapDegrees(720.25f) == 0.25f, "여러 바퀴도 나머지");
    check(wrapDegrees(-1.5f) == 358.5f, "음수는 양수 쪽으로 접는다");
    check(wrapDegrees(-360.f) == 0.f, "-360은 0");
    check(!std::signbit(wrapDegrees(-720.f)), "-720은 -0이 아니라 +0");

    // -1e-6 + 360은 float에서 정확히 360으로 반올림된다. 360이 아니라 0이어야 한다.
    check(wrapDegrees(-1e-6f) == 0.f, "반올림으로 360이 되면 0");

    // 유한하지 않은 값은 0으로 본다.
    check(wrapDegrees(std::numeric_limits<float>::quiet_NaN()) == 0.f, "NaN은 0");
    check(wrapDegrees(std::numeric_limits<float>::infinity()) == 0.f, "무한대는 0");
    check(wrapDegrees(-std::numeric_limits<float>::infinity()) == 0.f, "음의 무한대도 0");

    // 몇 바퀴 회전했든 같은 각도가 나온다.
    check(wrapDegrees(30.f + 360.f * 1000.f) == wrapDegrees(30.f), "1000바퀴 회전해도 같은 각도");
    check(wrapDegrees(10000030.f) == 310.f, "1e7도 근처도 정확히 접는다");
    check(wrapDegrees(-10000030.f) == 50.f, "-1e7도 근처도 양수 쪽으로 접는다");

    return meno::test::report();
}