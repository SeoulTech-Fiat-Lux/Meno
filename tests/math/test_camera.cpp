// 카메라 수학(Camera2D와 그 자유 함수들) 검사.
//
// 헤더만으로 성립하므로 SFML 빌드도 창도 없이 돈다.
// Renderer의 카메라 계열 함수는 전부 이 함수들에 위임하므로, 회전·줌·뷰포트·리사이즈의 규칙은 여기서 검사한다.
// 같은 규칙으로 SFML이 실제로 그리는지는 tests/graphics/test_camera_parity.cpp가 맞춰 본다.

#include <meno/math/Camera2D.hpp>

#include "check.hpp"

#include <cmath>
#include <initializer_list>

using namespace meno;

// --- 컴파일 타임 검사 -------------------------------------------------------

// 지정 초기화는 선언 순서를 따른다. 새 필드(viewport, fit)가 맨 끝에 있어야 기존 코드가 깨지지 않는다.
// 아래 줄이 컴파일되지 않으면 필드 순서가 바뀐 것이다.
constexpr Camera2D kAllFields{.center = {1.f, 2.f},
                              .size = {320.f, 180.f},
                              .zoom = 2.f,
                              .rotation = 30.f,
                              .viewport = {0.f, 0.f, 0.5f, 1.f},
                              .fit = CameraFit::Letterbox};
static_assert(kAllFields.fit == CameraFit::Letterbox);

// 기본값: 창 전체, Stretch, 크기를 정하지 않음(1픽셀 = 1월드유닛).
static_assert(Camera2D{}.viewport.size.x == 1.f && Camera2D{}.viewport.size.y == 1.f);
static_assert(Camera2D{}.fit == CameraFit::Stretch);

// --- 런타임 검사 ------------------------------------------------------------

namespace {

using meno::test::check;

bool near(float a, float b, float tolerance = 1e-3f) { return std::fabs(a - b) < tolerance; }
bool near(Vec2f a, Vec2f b, float tolerance = 1e-3f) {
    return near(a.x, b.x, tolerance) && near(a.y, b.y, tolerance);
}
bool same(const Rectf& a, const Rectf& b) { return a.position == b.position && a.size == b.size; }
bool finite(Vec2f v) { return std::isfinite(v.x) && std::isfinite(v.y); }

// 경계 위의 점도 포함으로 본다. Rect::contains는 오른쪽/아래 경계를 제외하는데,
// 회전된 화면의 모서리는 정확히 그 경계 위에 떨어지기 때문이다.
bool containsInclusive(const Rectf& rect, Vec2f point, float tolerance = 1e-2f) {
    return point.x >= rect.left() - tolerance && point.x <= rect.right() + tolerance &&
           point.y >= rect.top() - tolerance && point.y <= rect.bottom() + tolerance;
}

constexpr Vec2u kScreen{800u, 600u};

void checkDefaults() {
    // size == {0,0}: 보이는 영역이 창 크기와 같아 1픽셀 = 1월드유닛.
    const Camera2D camera{.center = {100.f, 50.f}};
    check(same(viewRect(camera, kScreen), Rectf{-300.f, -250.f, 800.f, 600.f}),
          "size {0,0}: 보이는 영역 = 창 크기, center가 가운데");
    check(near(worldToScreen(camera, kScreen, {100.f, 50.f}), Vec2f{400.f, 300.f}), "center는 화면 가운데에 온다");
    check(near(worldToScreen(camera, kScreen, {110.f, 50.f}), Vec2f{410.f, 300.f}), "월드 10 = 10픽셀");

    // 화면 좌표 카메라: center를 창 크기의 절반에 두면 픽셀 좌표와 월드 좌표가 같다. (Renderer::resetCamera)
    const Camera2D screen{.center = static_cast<Vec2f>(kScreen) / 2.f};
    check(near(screenToWorld(screen, kScreen, {0.f, 0.f}), Vec2f{0.f, 0.f}) &&
              near(screenToWorld(screen, kScreen, {799.f, 599.f}), Vec2f{799.f, 599.f}),
          "화면 좌표 카메라: 픽셀 좌표 == 월드 좌표");

    // 정수로 자르지 않는다. 수정 전 Renderer는 static_cast<int>로 0 방향 절삭을 해서 -0.5가 0이 되었다.
    check(near(screenToWorld(screen, kScreen, {-0.5f, 10.25f}), Vec2f{-0.5f, 10.25f}),
          "screenToWorld는 소수와 음수를 그대로 유지한다 (-0.5 -> -0.5)");
    check(near(worldToScreen(screen, kScreen, {0.25f, -3.75f}), Vec2f{0.25f, -3.75f}),
          "worldToScreen은 정수로 자르지 않는다");

    // 창 크기가 바뀌면 같은 카메라라도 결과가 새 크기를 따른다. (리사이즈 대응의 수학 쪽 절반)
    check(same(viewRect(Camera2D{}, {1280u, 720u}), Rectf{-640.f, -360.f, 1280.f, 720.f}),
          "size {0,0}: 창 크기가 바뀌면 보이는 영역도 따라 바뀐다");
}

void checkSizeAndZoom() {
    // size를 정하면 창 크기와 상관없이 그만큼의 월드가 보인다.
    const Camera2D fixed{.size = {320.f, 180.f}};
    check(near(viewRect(fixed, kScreen).size, Vec2f{320.f, 180.f}) &&
              near(viewRect(fixed, {1920u, 1080u}).size, Vec2f{320.f, 180.f}),
          "size를 정하면 창 크기와 상관없이 같은 영역이 보인다");

    // zoom은 size 위에 곱해진다. 보이는 영역 = size / zoom.
    check(near(viewRect({.size = {320.f, 180.f}, .zoom = 2.f}, kScreen).size, Vec2f{160.f, 90.f}),
          "zoom 2: 보이는 영역이 절반 (size / zoom)");
    check(near(viewRect({.zoom = 0.5f}, kScreen).size, Vec2f{1600.f, 1200.f}), "zoom 0.5: 보이는 영역이 두 배");

    // 0 이하, NaN, 무한대는 1로 본다. 0으로 나누거나 NaN이 퍼지지 않는다.
    bool zoomGuarded = true;
    for (const float zoom : {0.f, -2.f, NAN, INFINITY}) {
        zoomGuarded = zoomGuarded && same(viewRect({.zoom = zoom}, kScreen), viewRect({}, kScreen));
    }
    check(zoomGuarded, "zoom이 0 이하, NaN, 무한대이면 1로 본다");

    // 한 축만 0이거나 음수, NaN인 size는 정하지 않은 것({0,0})과 같다.
    bool sizeGuarded = true;
    for (const Vec2f size : {Vec2f{320.f, 0.f}, Vec2f{-320.f, 180.f}, Vec2f{NAN, 180.f}}) {
        sizeGuarded = sizeGuarded && same(viewRect({.size = size}, kScreen), viewRect({}, kScreen));
    }
    check(sizeGuarded, "size가 잘못되면 창 크기를 쓴다");
}

void checkRotation() {
    // 화면 위쪽(가운데에서 한 픽셀 위)이 가리키는 월드 방향. Camera2D::rotation 주석의 표와 같아야 한다.
    const Vec2f up = static_cast<Vec2f>(kScreen) / 2.f - Vec2f{0.f, 1.f};
    struct Case {
        float rotation;
        Vec2f direction;
        const char* what;
    };
    for (const Case& c : {Case{0.f, {0.f, -1.f}, "rotation 0: 화면 위쪽 = 월드 -y"},
                          Case{90.f, {1.f, 0.f}, "rotation 90: 화면 위쪽 = 월드 +x"},
                          Case{180.f, {0.f, 1.f}, "rotation 180: 화면 위쪽 = 월드 +y"},
                          Case{-90.f, {-1.f, 0.f}, "rotation -90: 화면 위쪽 = 월드 -x"}}) {
        check(near(screenToWorld({.rotation = c.rotation}, kScreen, up), c.direction), c.what);
    }

    // 카메라를 시계 방향으로 돌리면 월드는 화면에서 반시계 방향으로 돈다.
    // 월드 +x(회전 전엔 화면 오른쪽)는 90도에서 화면 위쪽으로 간다. 시계 방향이었다면 아래쪽으로 갔다.
    const Vec2f rightOfCenter = worldToScreen({.rotation = 90.f}, kScreen, {1.f, 0.f});
    check(near(rightOfCenter, Vec2f{400.f, 299.f}), "카메라 +90: 월드 +x가 화면 위쪽으로 (월드는 반시계로 돈다)");

    // 90도 돌리면 월드에서 보이는 폭과 높이가 뒤바뀐다.
    check(near(visibleWorldBounds({.rotation = 90.f}, kScreen).size, Vec2f{600.f, 800.f}, 1e-2f),
          "90도 회전: 보이는 폭과 높이가 바뀐다");

    // 45도면 두 축 모두 (w + h) / √2. 정사각형일 때만 정확히 √2배다.
    const float diagonal = (800.f + 600.f) / std::sqrt(2.f);
    check(near(visibleWorldBounds({.rotation = 45.f}, kScreen).size, Vec2f{diagonal, diagonal}, 1e-2f),
          "45도 회전: 보이는 영역이 축마다 (w + h) / √2");
    check(near(visibleWorldBounds({.size = {100.f, 100.f}, .rotation = 45.f}, kScreen).size,
               Vec2f{100.f * std::sqrt(2.f), 100.f * std::sqrt(2.f)}, 1e-2f),
          "45도 회전, 정사각형: 정확히 √2배");

    // 회전한 화면의 네 모서리에 비친 월드 좌표가 전부 visibleWorldBounds 안에 있어야 컬링이 물체를 놓치지 않는다.
    bool cornersInside = true;
    for (float rotation = -720.f; rotation <= 720.f; rotation += 7.5f) {
        const Camera2D camera{.center = {35.f, -12.f}, .zoom = 1.3f, .rotation = rotation};
        const Rectf bounds = visibleWorldBounds(camera, kScreen);
        for (const Vec2f corner : {Vec2f{0.f, 0.f}, Vec2f{800.f, 0.f}, Vec2f{0.f, 600.f}, Vec2f{800.f, 600.f}}) {
            cornersInside = cornersInside && containsInclusive(bounds, screenToWorld(camera, kScreen, corner));
        }
    }
    check(cornersInside, "모든 회전 각도에서 화면 모서리의 월드 좌표가 visibleWorldBounds 안에 있다");

    // 몇 바퀴를 돌았든 같은 결과. 접지 않고 라디안으로 바꾸면 1e7도 근처에서 몇 픽셀씩 어긋난다.
    // 10000170 = 360 * 27778 + 90. float로 정확히 표현되는 정수다.
    const Vec2f probe{123.f, -45.f};
    check(worldToScreen({.rotation = 10000170.f}, kScreen, probe) == worldToScreen({.rotation = 90.f}, kScreen, probe),
          "큰 각도: 360으로 접은 각도와 결과가 정확히 같다");
    check(worldToScreen({.rotation = -90.f}, kScreen, probe) == worldToScreen({.rotation = 270.f}, kScreen, probe),
          "음수 각도: -90 == 270");
    check(near(worldToScreen({.rotation = NAN}, kScreen, probe), worldToScreen({}, kScreen, probe)),
          "NaN 회전은 0으로 본다");
}

void checkViewport() {
    // 왼쪽 절반. size가 {0,0}이면 창이 아니라 뷰포트의 픽셀 크기를 쓴다.
    // 창 크기를 쓰면 반쪽 화면에 창 전체 폭이 들어가 가로로 2:1 찌그러진다.
    const Camera2D left{.viewport = {0.f, 0.f, 0.5f, 1.f}};
    check(same(viewportPixels(left, kScreen), Rectf{0.f, 0.f, 400.f, 600.f}), "뷰포트 픽셀 = 비율 * 창 크기");
    check(near(viewRect(left, kScreen).size, Vec2f{400.f, 600.f}), "size {0,0}: 보이는 영역 = 뷰포트의 픽셀 크기");

    // 변환은 뷰포트 기준이다. center는 뷰포트의 가운데로 간다.
    const Camera2D right{.center = {10.f, 20.f}, .viewport = {0.5f, 0.f, 0.5f, 1.f}};
    check(near(worldToScreen(right, kScreen, {10.f, 20.f}), Vec2f{600.f, 300.f}), "center는 뷰포트의 가운데로 간다");
    check(near(screenToWorld(right, kScreen, {400.f, 0.f}), Vec2f{-190.f, -280.f}),
          "뷰포트 좌상단 픽셀 = 보이는 영역의 좌상단");

    // 3등분. 모서리를 반올림하므로 맞닿은 화면 사이에 틈도 겹침도 없다.
    // SFML처럼 위치와 크기를 따로 반올림하면 640 / 3에서 [0,213) [213,426) [427,640)이 되어 426열이 빈다.
    constexpr Vec2u kOdd{640u, 480u};
    const float third = 1.f / 3.f;
    const Rectf a = viewportPixels({.viewport = {0.f, 0.f, third, 1.f}}, kOdd);
    const Rectf b = viewportPixels({.viewport = {third, 0.f, third, 1.f}}, kOdd);
    const Rectf c = viewportPixels({.viewport = {2.f * third, 0.f, third, 1.f}}, kOdd);
    check(a.left() == 0.f && a.right() == b.left() && b.right() == c.left() && c.right() == 640.f,
          "3등분 뷰포트: 맞닿은 경계가 같아 틈이 없다");

    bool integral = true;
    for (const Rectf& r : {a, b, c}) {
        integral = integral && r.left() == std::round(r.left()) && r.size.x == std::round(r.size.x);
    }
    check(integral, "뷰포트 픽셀은 항상 정수 값이다");

    // 크기 0인 뷰포트: 아무것도 그려지지 않고, 0으로 나누지도 않는다.
    const Camera2D empty{.center = {5.f, 6.f}, .viewport = {0.25f, 0.25f, 0.f, 0.f}};
    check(viewportPixels(empty, kScreen).size == Vec2f{0.f, 0.f}, "크기 0 뷰포트는 빈 사각형");
    check(screenToWorld(empty, kScreen, {1.f, 1.f}) == Vec2f{5.f, 6.f}, "빈 뷰포트: screenToWorld는 center");
    check(finite(worldToScreen(empty, kScreen, {1.f, 1.f})) && finite(viewRect(empty, kScreen).size),
          "빈 뷰포트: NaN이나 무한대가 나오지 않는다");

    // 잘못된 뷰포트(NaN, 음수 크기)도 빈 사각형이 되어 아무것도 그리지 않는다.
    check(viewportPixels({.viewport = {NAN, 0.f, 1.f, 1.f}}, kScreen).size == Vec2f{0.f, 0.f},
          "NaN 뷰포트는 빈 사각형");
    check(viewportPixels({.viewport = {0.5f, 0.f, -0.5f, 1.f}}, kScreen).size.x == 0.f, "음수 크기 뷰포트는 빈 사각형");

    // 창 크기가 0(최소화 등)이어도 NaN이 나오지 않는다.
    check(finite(worldToScreen({}, {0u, 0u}, {1.f, 1.f})) && finite(screenToWorld({}, {0u, 0u}, {1.f, 1.f})) &&
              finite(visibleWorldBounds({}, {0u, 0u}).size),
          "창 크기 {0,0}: NaN이나 무한대가 나오지 않는다");
}

void checkLetterbox() {
    // 16:9 설계 해상도를 정사각형 창에: 위아래에 띠가 생긴다. (레터박스)
    const Camera2D wide{.size = {320.f, 180.f}, .fit = CameraFit::Letterbox};
    check(same(viewportPixels(wide, {800u, 800u}), Rectf{0.f, 175.f, 800.f, 450.f}),
          "Letterbox: 가로가 긴 화면은 위아래에 띠");

    // 세로로 긴 설계 해상도를 가로로 긴 창에: 좌우에 띠가 생긴다. (필러박스)
    const Camera2D tall{.size = {100.f, 200.f}, .fit = CameraFit::Letterbox};
    check(same(viewportPixels(tall, kScreen), Rectf{250.f, 0.f, 300.f, 600.f}), "Letterbox: 세로가 긴 화면은 좌우에 띠");

    // 띠 안쪽에서는 비율이 유지된다. 월드 1유닛이 가로세로 같은 픽셀 수.
    const Vec2f origin = worldToScreen(wide, {800u, 800u}, {0.f, 0.f});
    const Vec2f unit = worldToScreen(wide, {800u, 800u}, {1.f, 1.f}) - origin;
    check(near(unit.x, unit.y) && near(origin, Vec2f{400.f, 400.f}), "Letterbox: 종횡비가 유지되고 center는 가운데");

    // Stretch(기본)는 뷰포트를 꽉 채운다.
    check(same(viewportPixels({.size = {320.f, 180.f}}, {800u, 800u}), Rectf{0.f, 0.f, 800.f, 800.f}),
          "Stretch: 뷰포트를 꽉 채운다");

    // size가 {0,0}이면 맞출 종횡비가 없으므로 Stretch와 같다.
    check(same(viewportPixels({.fit = CameraFit::Letterbox}, kScreen), Rectf{0.f, 0.f, 800.f, 600.f}),
          "Letterbox + size {0,0} == Stretch");

    // 뷰포트 안에서 레터박스한다. 미니맵처럼 창 일부에 고정 비율로 그릴 때.
    const Camera2D minimap{.size = {100.f, 100.f}, .viewport = {0.75f, 0.f, 0.25f, 0.5f}, .fit = CameraFit::Letterbox};
    check(same(viewportPixels(minimap, kScreen), Rectf{600.f, 50.f, 200.f, 200.f}),
          "Letterbox는 창이 아니라 뷰포트 안에서 맞춘다");
}

void checkRoundTrip() {
    // worldToScreen과 screenToWorld는 서로의 역함수다. 줌·회전·뷰포트·레터박스를 섞어도 원래 값으로 돌아온다.
    const Camera2D cameras[] = {
        {},
        {.center = {-250.f, 75.f}, .zoom = 3.f, .rotation = 33.f},
        {.center = {10.f, 10.f}, .size = {320.f, 180.f}, .rotation = -140.f, .fit = CameraFit::Letterbox},
        {.center = {1000.f, -500.f}, .zoom = 0.25f, .rotation = 721.f, .viewport = {0.1f, 0.2f, 0.3f, 0.6f}},
    };
    bool roundTrip = true;
    for (const Camera2D& camera : cameras) {
        for (const Vec2f world : {Vec2f{0.f, 0.f}, Vec2f{-123.5f, 456.25f}, Vec2f{999.f, -1.f}}) {
            const Vec2f back = screenToWorld(camera, kScreen, worldToScreen(camera, kScreen, world));
            roundTrip = roundTrip && near(back, world, 1e-2f);
        }
    }
    check(roundTrip, "worldToScreen -> screenToWorld 왕복이 원래 값으로 돌아온다");
}

} // namespace

int main() {
    checkDefaults();
    checkSizeAndZoom();
    checkRotation();
    checkViewport();
    checkLetterbox();
    checkRoundTrip();
    return meno::test::report();
}