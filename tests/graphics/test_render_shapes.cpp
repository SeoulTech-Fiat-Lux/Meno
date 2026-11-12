// 기본 도형(사각형, 원, 선, 다각형, 부채꼴)이 실제로 칠하는 픽셀 검사.
//
// 도형이 어디에 놓이는지(position, origin, rotation, 외곽선 방향)는 그려진 픽셀로만 확인할 수 있다.
// SFML의 래스터화 자체는 검사하지 않는다. 각 도형이 Renderer.hpp와 DrawParams.hpp의 약속대로
// 놓이는지만 본다.

// 이 테스트를 위해서는 창과 GL 컨텍스트가 필요하기 때문에, test_render와 같이
// MENO_DISPLAY_TESTS=1일 때만 돈다. 픽셀을 읽기 위해 내부 헤더(sfml_backend/Access.hpp)로
// sf::RenderWindow를 꺼낸다. 렌더링 자체는 공개 API(Renderer)로만 한다.

//     MENO_DISPLAY_TESTS=1 xvfb-run -a ctest --test-dir build -R render_shapes --output-on-failure

#include <meno/core/Window.hpp>
#include <meno/graphics/Renderer.hpp>

#include "sfml_backend/Access.hpp"
#include "sfml_backend/Convert.hpp"

#include "check.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string_view>

using namespace meno;

namespace {

using meno::test::check;

// tests/CMakeLists.txt의 SKIP_RETURN_CODE와 같아야 한다.
constexpr int kSkipReturnCode = 77;

constexpr Color kClear = colors::Black;
constexpr Color kInk = colors::Red;

// 창 크기에서 가운데를 계산한다. 둘을 따로 적으면 한쪽만 고쳤을 때 도형이 창 밖으로 나간다.
constexpr Vec2u kWindowSize{320u, 180u};
constexpr Vec2f kCenter = static_cast<Vec2f>(kWindowSize) / 2.f;

std::string_view env(const char* name) {
#ifdef _MSC_VER
#pragma warning(suppress : 4996) // getenv는 안전하지 않다고 경고한다. 이건 테스트용이므로 무시.
#endif
    const char* value = std::getenv(name);
    return value != nullptr ? value : "";
}

// 지금까지 그린 백 버퍼를 읽는다. endFrame(= display)으로 버퍼를 바꾸기 전에 불러야 한다.
sf::Image capture(Window& window) {
    sf::RenderWindow& native = *backend::WindowAccess::native(window);
    sf::Texture texture{native.getSize()};
    texture.update(native);
    return texture.copyToImage();
}

// 한 프레임을 그리고 그 결과를 읽는다.
template <typename Draw> sf::Image drawAndCapture(Window& window, Renderer& renderer, Draw draw) {
    renderer.beginFrame(kClear);
    draw();
    const sf::Image image = capture(window);
    renderer.endFrame();
    return image;
}

// 점 point가 들어 있는 픽셀의 색이 color인지를 확인. 창 밖의 점은 늘 false다.
// getPixel은 범위를 넘으면 Debug에서는 assert로 죽고, Release에서는 엉뚱한 메모리를 읽는다.
bool isColor(const sf::Image& image, Vec2f point, Color color) {
    const sf::Vector2u size = image.getSize();
    if (!(point.x >= 0.f && point.y >= 0.f && point.x < static_cast<float>(size.x) &&
          point.y < static_cast<float>(size.y))) {
        return false;
    }
    const sf::Vector2u pixel{static_cast<unsigned int>(point.x),
                             static_cast<unsigned int>(point.y)};
    return image.getPixel(pixel) == backend::toSf(color);
}

// center에서 degrees 방향(0도 = 오른쪽, 시계 방향)으로 distance만큼 떨어진 점.
Vec2f polar(Vec2f center, float degrees, float distance) {
    const float radians = degrees * 3.14159265f / 180.f;
    return center + Vec2f{std::cos(radians), std::sin(radians)} * distance;
}

// --- 사각형 -----------------------------------------------------------------

void checkRect(Window& window, Renderer& renderer) {
    // rect.position은 왼쪽 위 모서리다.
    sf::Image image = drawAndCapture(window, renderer, [&] {
        renderer.drawRect({{100.f, 50.f}, {40.f, 20.f}}, {.fill = kInk});
    });
    check(isColor(image, {102.f, 52.f}, kInk) && isColor(image, {137.f, 67.f}, kInk),
          "drawRect: position부터 size만큼 칠한다");
    check(isColor(image, {97.f, 60.f}, kClear) && isColor(image, {143.f, 60.f}, kClear) &&
              isColor(image, {120.f, 47.f}, kClear) && isColor(image, {120.f, 73.f}, kClear),
          "drawRect: 사각형 바깥은 그대로다");

    // origin은 rect.position에 놓일 로컬 점이자 회전의 중심점이다.
    // 가운데를 주면 rect.position이 중심이된다.
    // ex. 40x20을 가운데 기준으로 90도 회전하면 20x40이 된다.
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawRect({kCenter, {40.f, 20.f}},
                          {.fill = kInk, .origin = {20.f, 10.f}, .rotation = 90.f});
    });
    check(isColor(image, kCenter + Vec2f{0.f, -17.f}, kInk) &&
              isColor(image, kCenter + Vec2f{0.f, 17.f}, kInk) &&
              isColor(image, kCenter + Vec2f{17.f, 0.f}, kClear),
          "drawRect: origin을 축으로 돈다");

    // 외곽선 두께의 수치가 양수면 바깥쪽, 음수면 안쪽으로 그려진다.
    const Rectf box{{140.f, 70.f}, {40.f, 40.f}};
    const Vec2f leftEdge{box.left(), box.center().y};
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawRect(box,
                          {.fill = colors::Transparent, .outline = kInk, .outlineThickness = 4.f});
    });
    check(isColor(image, leftEdge - Vec2f{2.f, 0.f}, kInk) &&
              isColor(image, leftEdge + Vec2f{2.f, 0.f}, kClear),
          "drawRect: 양수 외곽선은 바깥쪽");
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawRect(box,
                          {.fill = colors::Transparent, .outline = kInk, .outlineThickness = -4.f});
    });
    check(isColor(image, leftEdge - Vec2f{2.f, 0.f}, kClear) &&
              isColor(image, leftEdge + Vec2f{2.f, 0.f}, kInk),
          "drawRect: 음수 외곽선은 안쪽");
}

// --- 원 ---------------------------------------------------------------------

void checkCircle(Window& window, Renderer& renderer) {
    constexpr float r = 30.f;

    // center는 원의 중심이다.
    // 바운딩 박스의 왼쪽 위가 기준인 sf::CircleShape와 맞추기 위해 drawCircle이 반지름만큼 옮긴다.
    sf::Image image =
        drawAndCapture(window, renderer, [&] { renderer.drawCircle(kCenter, r, {.fill = kInk}); });
    bool round = true;
    for (float deg = 0.f; deg < 360.f; deg += 45.f) {
        round = round && isColor(image, polar(kCenter, deg, r - 3.f), kInk) &&
                isColor(image, polar(kCenter, deg, r + 3.f), kClear);
    }
    check(round, "drawCircle: center를 중심으로 반지름만큼 칠한다");

    // params.origin은 원의 중심 기준 로컬 점이며 center에 놓인다.
    // rotation은 center를 축으로 돈다.
    constexpr float small = 15.f;
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawCircle(kCenter, small, {.fill = kInk, .origin = {30.f, 0.f}});
    });
    check(isColor(image, kCenter - Vec2f{30.f, 0.f}, kInk) && isColor(image, kCenter, kClear),
          "drawCircle: 원의 중심은 center - origin에 놓인다");
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawCircle(kCenter, small,
                            {.fill = kInk, .origin = {30.f, 0.f}, .rotation = 90.f});
    });
    check(isColor(image, kCenter - Vec2f{0.f, 30.f}, kInk) &&
              isColor(image, kCenter - Vec2f{30.f, 0.f}, kClear),
          "drawCircle: rotation은 center를 축으로 돈다");
}

// --- 선 ---------------------------------------------------------------------

void checkLine(Window& window, Renderer& renderer) {
    // 두께는 선을 가운데에 두고 양쪽으로 반씩 퍼진다.
    // ex. y 90에서 두께가 6인 가로선은, y 87~93을 칠한다.
    sf::Image image = drawAndCapture(
        window, renderer, [&] { renderer.drawLine({100.f, 90.f}, {220.f, 90.f}, kInk, 6.f); });
    check(isColor(image, {160.f, 88.f}, kInk) && isColor(image, {160.f, 92.f}, kInk) &&
              isColor(image, {160.f, 85.f}, kClear) && isColor(image, {160.f, 95.f}, kClear),
          "drawLine: 두께가 선 양쪽으로 반씩 퍼진다");
    check(isColor(image, {97.f, 90.f}, kClear) && isColor(image, {223.f, 90.f}, kClear),
          "drawLine: from에서 시작해 to에서 끝난다");

    // 방향은 atan2로 구해 사각형을 돌린다. 아래에서 위로 긋는 세로선도 from에서 to로 뻗어야 한다.
    image = drawAndCapture(window, renderer,
                           [&] { renderer.drawLine({160.f, 140.f}, {160.f, 40.f}, kInk, 4.f); });
    check(isColor(image, {160.f, 90.f}, kInk) && isColor(image, {165.f, 90.f}, kClear) &&
              isColor(image, {160.f, 37.f}, kClear) && isColor(image, {160.f, 143.f}, kClear),
          "drawLine: 세로선도 from에서 to로 뻗는다");

    // 길이 0인 선은 두께가 있어도 그리지 않는다.
    image =
        drawAndCapture(window, renderer, [&] { renderer.drawLine(kCenter, kCenter, kInk, 10.f); });
    check(isColor(image, kCenter, kClear), "drawLine: 길이 0은 그리지 않는다");
}

// --- 다각형 -----------------------------------------------------------------

void checkPolygon(Window& window, Renderer& renderer) {
    // 중괄호 목록 오버로드. 점은 position 기준 로컬 좌표다.
    sf::Image image = drawAndCapture(window, renderer, [&] {
        renderer.drawPolygon(kCenter, {{0.f, -30.f}, {30.f, 20.f}, {-30.f, 20.f}}, {.fill = kInk});
    });
    check(isColor(image, kCenter + Vec2f{0.f, 5.f}, kInk), "drawPolygon: 삼각형 안쪽이 칠해진다");
    check(isColor(image, kCenter + Vec2f{25.f, -20.f}, kClear) &&
              isColor(image, {5.f, 5.f}, kClear),
          "drawPolygon: 삼각형 바깥은 그대로다");

    // rotation은 position을 축으로 돈다.
    // 오른쪽으로 뻗은 막대를 시계방향으로 90도 돌리면 아래로 뻗는다.
    const std::array<Vec2f, 4> bar{{{0.f, -3.f}, {40.f, -3.f}, {40.f, 3.f}, {0.f, 3.f}}};
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawPolygon(kCenter, bar, {.fill = kInk, .rotation = 90.f});
    });
    check(isColor(image, kCenter + Vec2f{0.f, 30.f}, kInk) &&
              isColor(image, kCenter + Vec2f{30.f, 0.f}, kClear),
          "drawPolygon: rotation은 position을 축으로 돈다");

    // origin은 position에 놓일 로컬 점이다. 막대의 가운데를 주면 가운데를 축으로 돈다.
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawPolygon(kCenter, bar, {.fill = kInk, .origin = {20.f, 0.f}, .rotation = 90.f});
    });
    check(isColor(image, kCenter + Vec2f{0.f, -15.f}, kInk) &&
              isColor(image, kCenter + Vec2f{0.f, 15.f}, kInk) &&
              isColor(image, kCenter + Vec2f{0.f, 25.f}, kClear),
          "drawPolygon: origin을 축으로 돈다");

    // 외곽선만 그린 울퉁불퉁한 10각형.
    // 꼭짓점이 36도 간격이고 반지름이 1.0과 0.8을 오가서, 0.8인 꼭짓점은 살짝 안으로 들어간 오목한 꼭짓점이다.
    // 모든 꼭짓점에서 외곽선이 바깥쪽으로 그려져야 한다.
    constexpr float rock = 50.f;
    std::array<Vec2f, 10> rockPoints{};
    for (std::size_t i = 0; i < rockPoints.size(); ++i) {
        const float scale = i % 2 == 0 ? 1.f : 0.8f;
        rockPoints[i] = polar({0.f, 0.f}, 36.f * static_cast<float>(i), rock * scale);
    }
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawPolygon(
            kCenter, rockPoints,
            {.fill = colors::Transparent, .outline = kInk, .outlineThickness = 3.f});
    });
    bool outward = isColor(image, kCenter, kClear);
    for (std::size_t i = 0; i < rockPoints.size(); ++i) {
        const float distance = rockPoints[i].length();
        const float deg = 36.f * static_cast<float>(i);
        outward = outward && isColor(image, polar(kCenter, deg, distance + 1.5f), kInk) &&
                  isColor(image, polar(kCenter, deg, distance - 2.f), kClear);
    }
    check(outward, "drawPolygon: 오목한 꼭짓점이 있어도 외곽선은 바깥쪽");

    // 점이 3개 미만이면 외곽선을 포함하여 그 무엇도 그리지 않는다.
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawPolygon(kCenter, {{0.f, 0.f}, {40.f, 40.f}},
                             {.fill = kInk, .outline = kInk, .outlineThickness = 5.f});
    });
    check(isColor(image, kCenter + Vec2f{20.f, 20.f}, kClear),
          "drawPolygon: 점 2개는 그리지 않는다");
}

// --- 부채꼴 -----------------------------------------------------------------

// start에서 sweep 방향으로 몇 도를 가야 degrees에 닿는지를 확인하는 함수 [0, 360)
float travel(float start, float sweep, float degrees) {
    const float d = std::fmod(sweep >= 0.f ? degrees - start : start - degrees, 360.f);
    return d < 0.f ? d + 360.f : d;
}

// apex를 꼭짓점으로 한 부채꼴(start, sweep)이 그려졌는지를 본다.
// kMargin 이내에 해당하는 경계 부근은 픽셀 값의 반올림 결과에 따라 갈리므로 건너뛴다.
// 반지름 바깥(r + 3)은 어느 방향이든 비어 있어야 한다.
bool isSector(const sf::Image& image, Vec2f apex, float r, float start, float sweep) {
    constexpr float kMargin = 6.f;
    const float span = std::fabs(sweep);
    const bool full = span >= 360.f;

    bool ok = true;
    for (float deg = 5.f; deg < 360.f; deg += 10.f) {
        ok = ok && isColor(image, polar(apex, deg, r + 3.f), kClear);

        const float t = travel(start, sweep, deg);
        const bool inside = full || t < span;
        const float toEdge =
            full ? 360.f : (inside ? std::min(t, span - t) : std::min(t - span, 360.f - t));
        if (toEdge < kMargin) {
            continue;
        }
        const Color expected = inside ? kInk : kClear;
        ok = ok && isColor(image, polar(apex, deg, r * 0.5f), expected) &&
             isColor(image, polar(apex, deg, r * 0.9f), expected);
    }

    // 좁은 부채꼴은 위의 표본이 모두 경계 근처라 하나도 남지 않을 수 있다.
    // 가운데 방향 하나는 늘 본다.
    if (!full) {
        ok = ok && isColor(image, polar(apex, start + sweep / 2.f, r * 0.8f), kInk);
    }
    return ok;
}

void checkSector(Window& window, Renderer& renderer) {
    constexpr float r = 50.f;

    // 게임에서 부채꼴이 쓰이는 모양을 각도 구간별로 하나씩 둔다.
    // 0도 = 오른쪽, 양수 sweepDeg = 시계 방향.
    struct Case {
        float start;
        float sweep;
        const char* what;
    };
    constexpr std::array<Case, 9> cases{{
        {-90.f, 90.f, "12시부터 시계 방향 90도 (쿨다운 1/4)"},
        {-90.f, -90.f, "12시부터 반시계 방향 90도 (음수 sweepDeg)"},
        {-30.f, 60.f, "오른쪽을 보는 60도 (시야, 공격 범위)"},
        {0.f, 180.f, "아래쪽 반원 (볼록과 오목의 경계)"},
        {30.f, 300.f, "300도 (180도를 넘는 오목한 부채꼴)"},
        {-90.f, -270.f, "반시계 방향 270도 (음수이면서 오목)"},
        {-90.f, 359.f, "359도 (한 바퀴 직전)"},
        {750.f, 60.f, "시작 각도 750도 (= 30도, 누적된 회전값)"},
        {0.f, 5.f, "5도 (변 하나짜리 좁은 부채꼴)"},
    }};
    for (const Case c : cases) {
        const sf::Image image = drawAndCapture(window, renderer, [&] {
            renderer.drawSector(kCenter, r, c.start, c.sweep, {.fill = kInk});
        });
        char what[128];
        std::snprintf(what, sizeof what, "drawSector: %s", c.what);
        check(isSector(image, kCenter, r, c.start, c.sweep), what);
    }

    // |sweepDeg| >= 360이면 원이다. 몇 바퀴든, 방향이 어느 쪽이든 같다.
    bool full = true;
    for (const float sweep : {360.f, 720.f, -360.f}) {
        const sf::Image image = drawAndCapture(
            window, renderer, [&] { renderer.drawSector(kCenter, r, 0.f, sweep, {.fill = kInk}); });
        full = full && isSector(image, kCenter, r, 0.f, 360.f);
    }
    check(full, "drawSector: |sweepDeg| >= 360은 원");

    // 채우기는 직접 그리므로 sf::Shape와 같은 변환을 받는지 따로 본다.
    sf::Image image = drawAndCapture(window, renderer, [&] {
        renderer.drawSector(kCenter, r, -30.f, 60.f, {.fill = kInk, .rotation = 90.f});
    });
    check(isSector(image, kCenter, r, 60.f, 60.f), "drawSector: rotation을 받으면 함께 돈다");

    // origin과 rotation은 drawCircle과 같은 자리로 옮긴다.
    // origin {30, 0}과 rotation 90이면 꼭짓점은 위쪽 30에 놓이고, 시작 방향도 90도 돈다.
    constexpr float small = 40.f;
    const ShapeParams moved{.fill = kInk, .origin = {30.f, 0.f}, .rotation = 90.f};
    const Vec2f apex = kCenter - Vec2f{0.f, 30.f};
    image = drawAndCapture(window, renderer,
                           [&] { renderer.drawSector(kCenter, small, 0.f, 359.f, moved); });
    const bool sectorMoved = isSector(image, apex, small, 90.f, 359.f);
    image = drawAndCapture(window, renderer, [&] { renderer.drawCircle(kCenter, small, moved); });
    check(sectorMoved && isSector(image, apex, small, 0.f, 360.f),
          "drawSector: origin과 rotation은 drawCircle과 같은 자리로 옮긴다");

    // 외곽선은 양수면 바깥쪽에 그려진다.
    // 앞의 셋(0/270, 35/320, 45/330)은 점을 중심부터 넘기면 sf::Shape가 방향을 거꾸로 읽어 안쪽으로 외곽선이 그려진다.
    // 나머지는 대조군이다.
    constexpr std::array<Case, 6> outlines{{
        {0.f, 270.f, ""},
        {35.f, 320.f, ""},
        {45.f, 330.f, ""},
        {0.f, 300.f, ""},
        {30.f, 300.f, ""},
        {100.f, 90.f, ""},
    }};
    for (const Case c : outlines) {
        const sf::Image outlined = drawAndCapture(window, renderer, [&] {
            renderer.drawSector(
                kCenter, r, c.start, c.sweep,
                {.fill = colors::Transparent, .outline = kInk, .outlineThickness = 4.f});
        });
        const float middle = c.start + c.sweep / 2.f; // 호의 가운데 방향
        char what[96];
        std::snprintf(what, sizeof what, "drawSector: 외곽선은 바깥쪽 (start %.0f, sweep %.0f)",
                      c.start, c.sweep);
        check(isColor(outlined, polar(kCenter, middle, r + 2.f), kInk) &&
                  isColor(outlined, polar(kCenter, middle, r - 2.f), kClear),
              what);
    }

    // 음수 두께는 안쪽 외곽선을 뜻한다.
    // 점 순서를 직접 정하므로 sf::RectangleShape처럼 저절로 맞는다고 가정하지 않는다.
    image = drawAndCapture(window, renderer, [&] {
        renderer.drawSector(
            kCenter, r, 30.f, 300.f,
            {.fill = colors::Transparent, .outline = kInk, .outlineThickness = -4.f});
    });
    check(isColor(image, polar(kCenter, 180.f, r - 2.f), kInk) &&
              isColor(image, polar(kCenter, 180.f, r + 2.f), kClear),
          "drawSector: 음수 외곽선은 안쪽");

    // sweepDeg 0이나 NaN, 반지름 0 이하나 무한대·NaN이면 외곽선까지 아무것도 그리지 않는다.
    // 남은 시간으로 계산한 반지름이 마지막 프레임에 음수로 넘어가도 이상한 도형이 남지 않는다.
    image = drawAndCapture(window, renderer, [&] {
        const ShapeParams loud{.fill = kInk, .outline = kInk, .outlineThickness = 4.f};
        renderer.drawSector(kCenter, r, 0.f, 0.f, loud);
        renderer.drawSector(kCenter, r, 0.f, std::nanf(""), loud);
        renderer.drawSector(kCenter, 0.f, 0.f, 90.f, loud);
        renderer.drawSector(kCenter, -r, 0.f, 90.f, loud);
        renderer.drawSector(kCenter, std::nanf(""), 0.f, 90.f, loud);
        renderer.drawSector(kCenter, INFINITY, 0.f, 90.f, loud);
    });
    check(isColor(image, kCenter, kClear) &&
              isColor(image, polar(kCenter, 45.f, r * 0.5f), kClear) &&
              isColor(image, polar(kCenter, 45.f, r + 1.f), kClear),
          "drawSector: sweepDeg 0·NaN, 반지름 0 이하·무한대·NaN은 그리지 않는다");
}

} // namespace

int main() {
    if (env("MENO_DISPLAY_TESTS").empty() || env("MENO_DISPLAY_TESTS") == "0") {
        std::printf("SKIP 창과 GL 컨텍스트가 필요한 테스트. MENO_DISPLAY_TESTS = 1일 때만 돈다.\n");
        return kSkipReturnCode;
    }

    Window window{kWindowSize, "meno render shapes test"};
    Renderer renderer{window};

    checkRect(window, renderer);
    checkCircle(window, renderer);
    checkLine(window, renderer);
    checkPolygon(window, renderer);
    checkSector(window, renderer);

    return meno::test::report();
}
