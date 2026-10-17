// 렌더 계층(Renderer, Texture, Font) 검사.

// meno 라이브러리(= SFML)를 링크하지만 공개 헤더만 include한다.

// 런타임 검사는 전부 디스플레이가 필요하다. Renderer는 Window 없이 만들 수 없고,
// sf::Texture는 기본 생성만 해도(sf::GlResource) 공유 GL 컨텍스트를 만들며,
// 글자 크기를 재려면 글리프를 GPU 텍스처(폰트 아틀라스)에 올려야 한다.
// 디스플레이가 없으면 SFML은 프로세스를 abort하므로, MENO_DISPLAY_TESTS=1일 때만
// 실제로 돌고 아니면 종료 코드 77(ctest의 "건너뜀")로 끝난다.
// 아래 static_assert는 컴파일 타임 검사라서 건너뛰더라도 빌드할 때 항상 검사된다.

//     MENO_DISPLAY_TESTS=1 ctest --test-dir build -R render --output-on-failure
//     MENO_DISPLAY_TESTS=1 xvfb-run -a ctest --test-dir build   # 디스플레이 없는 Linux

// 폰트는 흔한 시스템 경로에서 찾는다. 없으면 MENO_TEST_FONT로 경로를 지정한다.

#include <meno/core/Window.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>

#include "check.hpp"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

using namespace meno;

// --- 컴파일 타임 검사 -------------------------------------------------------

// Renderer는 Window를 빌려 쓰는 타입이므로 복사와 이동을 막는다.
// 이동다한 Renderer(impl_ == nullptr)가 생길 수 없어야, 맴버 함수들이 검사 없이 역참조해도 안전하다.
static_assert(!std::is_copy_constructible_v<Renderer>);
static_assert(!std::is_copy_assignable_v<Renderer>);
static_assert(!std::is_move_constructible_v<Renderer>);
static_assert(!std::is_move_assignable_v<Renderer>);

// Texture/Font는 ResourceManager가 컨테이너에 옮겨 담을 수 있어야 한다.
// noexcept가 빠지면 std::vector가 재할당할 때 이동 대신 복사를 시도하다 컴파일 에러가 난다.
static_assert(!std::is_copy_constructible_v<Texture>);
static_assert(std::is_nothrow_move_constructible_v<Texture>);
static_assert(std::is_nothrow_move_assignable_v<Texture>);
static_assert(!std::is_copy_constructible_v<Font>);
static_assert(std::is_nothrow_move_constructible_v<Font>);
static_assert(std::is_nothrow_move_assignable_v<Font>);

// --- 런타임 검사 ------------------------------------------------------------

namespace {
using meno:: test::check;

// tests/CMakeLists.txt의 SKIP_RETURN_CODE와 같아야 한다.
constexpr int kSkipReturnCode = 77;

bool near(float a, float b, float tolerance = 1e-3f) { return std::fabs(a - b) < tolerance; }
bool near(Vec2f a, Vec2f b, float tolerance = 1e-3f) {
    return near(a.x, b.x, tolerance) && near(a.y, b.y, tolerance);
}

// 경계 위의 점도 포함으로 본다. Rect::contains는 오른쪽/아래 경계를 제외하는데,
// 회전된 화면의 모서리는 정확히 그 경계 위에 떨어지기 때문이다.
bool containsInclusive(const Rectf& rect, Vec2f point, float tolerance = 0.5f) {
    return point.x >= rect.left() - tolerance && point.x <= rect.right() + tolerance &&
           point.y >= rect.top() - tolerance && point.y <= rect.bottom() + tolerance;
}

// 환경 변수. 없으면 빈 문자열.
std::string_view env(const char* name) {
#ifdef _MSC_VER
#pragma warning(suppress : 4996) // getenv는 안전하지 않다고 경고한다. 이건 테스트용이므로 무시.
#endif
    const char* value = std::getenv(name);
    return value != nullptr ? value : "";
}

std::optional<Font> loadTestFont() {
    const std::array<std::string_view, 5> candidates{
        env("MENO_TEST_FONT"),  // 환경 변수로 지정한 폰트
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",   // Ubuntu (fonts-dejavu-core)
        "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",  // macOS
        "/Library/Fonts/Arial.ttf",
    };

    for (const std::string_view path : candidates) {
        if (path.empty() || !std::filesystem::exists(path)) {
            continue;
        }
        if (auto font = Font::loadFromFile(path)) {
            std::printf("   (폰트: %.*s)\n", static_cast<int>(path.size()), path.data());
            return font;
        }
    }
    return std::nullopt;
}

// --- 이동당한 객체 -----------------------------------------------------------

void checkMovedFrom(Renderer& renderer) {
    // 이동당한 Window로는 Renderer를 만들 수 없다. 받아 두면 첫 draw에서 널 역참조가 난다.
    Window source{{64u, 64u}, "moved-from"};
    const Window owner = std::move(source);

    bool threw = false;
    try {
        const Renderer invalid{source};
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "이동당한 Window로 Renderer를 만들면 std::invalid_argument");

    // 이동당한 Window 자체도 크래시 없이 닫힌 창처럼 행동한다.
    check(!source.isOpen() && source.size() == Vec2u{0u, 0u}, "이동당한 Window는 닫힌 창처럼 보인다");
    source.pollEvents();
    source.close();

    // 이동당한 Texture는 조회해도 크래시 없이 빈 텍스처처럼 행동한다(docs/render-api.md §4).
    // 수정 전에는 size()에서 널 역참조로 죽었다. 예제가 checker.size()를 부르므로 실제 경로다.
    Texture texture;
    Texture keepTexture = std::move(texture);
    check(texture.size() == Vec2u{0u, 0u}, "이동당한 Texture의 size()는 {0,0}");
    texture.setSmooth(true); // 무시되어야 한다
    check(!texture.isSmooth(), "이동당한 Texture의 setSmooth()는 무시되고 isSmooth()는 false");

    // 빈 껍데기에 새 Texture를 이동 대입하면 다시 쓸 수 있다.
    // 이번엔 keepTexture가 빈 껍데기가 된다.
    texture = std::move(keepTexture);
    texture.setSmooth(true);
    check(texture.isSmooth(), "이동당한 Texture에 이동 대입하면 다시 쓸 수 있다");

    // 이동당한 Texture/Font로 그리거나 재도 크래시하지 않는다.
    Font font;
    const Font keepFont = std::move(font);

    renderer.beginFrame();
    renderer.draw(keepTexture, {10.f, 10.f});
    renderer.drawText(font, "moved", {10.f, 10.f});
    renderer.endFrame();

    check(renderer.measureText(font, "moved") == Vec2f{0.f, 0.f}, "이동당한 Font로 재면 {0,0}");
    const Rectf bounds = renderer.measureTextBounds(font, "moved");
    check(bounds.position == Vec2f{0.f, 0.f} && bounds.size == Vec2f{0.f, 0.f},
          "이동당한 Font로 measureTextBounds하면 빈 사각형");
}

// --- 텍스트 측정 -------------------------------------------------------------

void checkText(const Renderer& renderer, const Font& font) {
    constexpr std::string_view text = "meno 2D";
    const TextParams plain{.characterSize = 24};
    const TextParams outlined{.characterSize = 24, .outlineThickness = 2.f};

    const Rectf a = renderer.measureTextBounds(font, text, plain);
    const Rectf b = renderer.measureTextBounds(font, text, outlined);
    check(a.size.x > 0.f && a.size.y > 0.f, "글자가 있으면 크기가 0보다 크다");

    // SFML은 외곽선 t가 있으면 바운드를 축마다 ceil(t)씩 양쪽으로 넓힌다.
    // 수정 전에는 측정용 sf::Text에 외곽선이 빠져 있어서 b == a였다.
    check(near(b.size, a.size + Vec2f{4.f, 4.f}), "외곽선 2를 주면 잰 크기가 축마다 4 커진다");
    check(near(b.position, a.position - Vec2f{2.f, 2.f}), "외곽선 2를 주면 바운드 원점이 2씩 당겨진다");

    check(renderer.measureText(font, text, outlined) == b.size, "measureText == measureTextBounds().size");

    // 바운드 원점은 {0,0}이 아니다(글자 위 여백). measureText만으로 정렬하면 이만큼 틀어진다.
    check(a.position.y > 0.f, "바운드 원점에 윗여백이 있다 (measureTextBounds가 필요한 이유)");

    // origin과 rotation은 적용 전의 로컬 좌표로 잰다.
    TextParams transformed = outlined;
    transformed.origin = {50.f, 10.f};
    transformed.rotation = 30.f;
    const Rectf c = renderer.measureTextBounds(font, text, transformed);
    check(c.position == b.position && c.size == b.size, "origin/rotation은 측정 결과에 영향을 주지 않는다");

    check(renderer.measureText(font, "", plain) == Vec2f{0.f, 0.f}, "빈 문자열의 크기는 {0,0}");
}

// --- 카메라 -----------------------------------------------------------------

void checkCamera(Renderer& renderer) {
    const auto screen = static_cast<Vec2f>(renderer.framebufferSize());

    // 회전이 없으면 center 기준으로 화면 크기 / zoom.
    renderer.setCamera({.center = {100.f, 50.f}, .zoom = 2.f});
    const Rectf flat = renderer.visibleWorldBounds();
    check(near(flat.size, screen / 2.f) && near(flat.center(), Vec2f{100.f, 50.f}),
          "회전 없는 카메라: 보이는 영역 = 화면 / zoom");

    // 90도 돌리면 월드에서 보이는 폭과 높이가 뒤바뀐다.
    renderer.setCamera({.rotation = 90.f});
    const Rectf quarter = renderer.visibleWorldBounds();
    check(near(quarter.size, Vec2f{screen.y, screen.x}, 0.01f), "90도 회전: 폭과 높이가 바뀐다");

    // 45도: 화면 네 모서리에 비치는 월드 좌표가 모두 영역 안에 있어야 한다.
    // 수정 전에는 회전을 무시한 사각형이 나와 모서리가 영역 밖으로 빠졌다
    // (= 그 결과로 컬링하면 화면 모서리에 보이는 오브젝트가 사라진다).
    renderer.setCamera({.rotation = 45.f});
    const Rectf tilted = renderer.visibleWorldBounds();
    bool allCornersInside = true;
    for (const Vec2f corner : {Vec2f{0.f, 0.f}, Vec2f{screen.x, 0.f}, Vec2f{0.f, screen.y}, screen}) {
        allCornersInside = allCornersInside && containsInclusive(tilted, renderer.screenToWorld(corner));
    }
    check(allCornersInside, "45도 회전: 화면 모서리의 월드 좌표가 모두 visibleWorldBounds 안에 있다");

    renderer.resetCamera();
}

}   // namespace

int main() {
    if (env("MENO_DISPLAY_TESTS").empty() || env("MENO_DISPLAY_TESTS") == "0") {
        std::printf("SKIP 창과 GL 컨텍스트가 필요한 테스트. MENO_DISPLAY_TESTS = 1일 때만 돈다.\n");
        return kSkipReturnCode;
    }

    Window window{{320u, 180u}, "meno render test"};
    Renderer renderer{window};

    checkMovedFrom(renderer);
    checkCamera(renderer);

    const std::optional<Font> font = loadTestFont();
    check(font.has_value(), "테스트 폰트를 찾았다. (없으면 MENO_TEST_FONT로 경로를 지정)");
    if (font) {
        checkText(renderer, *font);
    }

    return meno::test::report();
}