// 창 크기가 바뀐 뒤의 Renderer와, 뷰포트가 실제로 그리는 픽셀 검사.
//
// 좌표 계산의 규칙은 tests/math/test_camera.cpp가, 그 규칙을 SFML이 따르는지는 test_camera_parity.cpp가 본다.
// 그러나 위 두 검사로는 아래 사안을 확인할 수 없으므로, test_render_resize.cpp를 따로 둔다:
//   1. 크키가 재설정된 Renderer가 여전히 기존 규칙을 잘 따르는가 (카메라 보관, beginFrame마다 재적용)
//   2. GPU가 그린 픽셀이 실제로 해당 위치에 있는가 (레터박스 띠, 3등분 화면의 틈, 빈 뷰포트)

// 이 테스트를 위해서는 창과 GL 컨텍스트가 필요하기 때문에, test_render와 같이 MENO_DISPLAY_TESTS=1일 때만 돈다.
// 창 크기를 바꾸고 픽셀을 읽기 위해 내부 헤더(sfml_backend/Access.hpp)로 sf::RenderWindow를 꺼낸다.
// 렌더링 자체는 공개 API(Renderer)로만 한다.

//     MENO_DISPLAY_TESTS=1 xvfb-run -a ctest --test-dir build -R render_resize --output-on-failure

#include <meno/core/Window.hpp>
#include <meno/graphics/Renderer.hpp>

#include "sfml_backend/Access.hpp"
#include "sfml_backend/Convert.hpp"

#include "check.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <thread>

using namespace meno;

namespace {

using meno::test::check;

// tests/CMakeLists.txt의 SKIP_RETURN_CODE와 같아야 한다.
constexpr int kSkipReturnCode = 77;

constexpr Color kClear = colors::Black;
constexpr Color kInk = colors::Red;

// 어떤 카메라로 봐도 보이는 영역을 전부 덮는 사각형.
const Rectf kEverywhere{-100000.f, -100000.f, 200000.f, 200000.f};

std::string_view env(const char* name) {
#ifdef _MSC_VER
#pragma warning(suppress : 4996) // getenv는 안전하지 않다고 경고한다. 이건 테스트용이므로 무시.
#endif
    const char* value = std::getenv(name);
    return value != nullptr ? value : "";
}

bool near(Vec2f a, Vec2f b, float tolerance = 1e-3f) {
    return std::fabs(a.x - b.x) < tolerance && std::fabs(a.y - b.y) < tolerance;
}

// 창 크기를 바꾸고, SFML이 Resized 이벤트를 받아 크기를 갱신할 때까지 pollEvents를 돌린다.
// SFML은 pollEvent 안에서 getSize()를 새 값으로 바꾼다. Renderer가 별도의 통지 없이 리사이즈를 따라가는 근거다.
bool resize(Window& window, Renderer& renderer, Vec2u size) {
    backend::WindowAccess::native(window)->setSize(backend::toSf(size));
    for (int i = 0; i < 200 && window.size() != size; ++i) {
        window.pollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 빈 프레임을 한 번 표시한다. 드라이버에 따라(Xvfb의 Mesa 등) GL 백 버퍼는 창 크기를 바꾼 뒤
    // 다음 버퍼 교체 때에야 새 크기로 바뀐다. 그 전에 읽으면 옛 크기의 버퍼를 읽게 된다.
    // 카메라가 옛 크기에 묶이는 버그는 이후 프레임에도 계속 남으므로, 한 프레임을 버려도 검사는 약해지지 않는다.
    renderer.beginFrame(kClear);
    renderer.endFrame();
    return window.size() == size;
}

// 지금까지 그린 백 버퍼를 읽는다. endFrame(= display)으로 버퍼를 바꾸기 전에 불러야 한다.
sf::Image capture(Window& window) {
    sf::RenderWindow& native = *backend::WindowAccess::native(window);
    sf::Texture texture{native.getSize()};
    texture.update(native);
    return texture.copyToImage();
}

bool isColor(const sf::Image& image, unsigned int x, unsigned int y, Color color) {
    return image.getPixel({x, y}) == backend::toSf(color);
}

// --- 화면 좌표(resetCamera)는 창 크기를 따라간다 ----------------------------

void checkScreenCoordinates(Window& window, Renderer& renderer) {
    // 처음 만든 Renderer와 resetCamera() 뒤는 화면 좌표 상태이며 camera()가 비어 있다.
    check(!renderer.camera().has_value(),
          "처음 만든 Renderer는 화면 좌표 상태 (camera() == nullopt)");

    // 리사이즈 전 SFML 기본 뷰(320x180)가 남아 있으면 화면이 두 배로 늘어난다.
    // 그러면 (100, 100)의 20x20 사각형은 (200, 200)의 40x40으로 그려지고,
    // 우하단 구석 (600, 320)의 사각형은 (1200, 640)으로 밀려 창 밖에 그려진다.
    renderer.beginFrame(kClear);
    renderer.drawRect({{100.f, 100.f}, {20.f, 20.f}}, {.fill = kInk});
    renderer.drawRect({{600.f, 320.f}, {20.f, 20.f}}, {.fill = kInk});
    const sf::Image image = capture(window);
    renderer.endFrame();

    check(isColor(image, 110, 110, kInk) && isColor(image, 230, 230, kClear),
          "리사이즈 후 화면 좌표: 화면이 늘어나지 않는다 (20x20은 20x20으로)");
    check(isColor(image, 610, 330, kInk),
          "리사이즈 후 화면 좌표: 우하단에 그린 사각형이 그 픽셀에 있다");
    check(near(renderer.screenToWorld({639.f, 359.f}), Vec2f{639.f, 359.f}),
          "리사이즈 후 화면 좌표: 픽셀 좌표 == 월드 좌표 (수정 전에는 {319.5, 179.5})");
    check(near(renderer.screenToWorld({-0.5f, 0.5f}), Vec2f{-0.5f, 0.5f}),
          "screenToWorld는 정수로 자르지 않는다 (수정 전에는 -0.5 -> 0)");
}

// --- 리사이즈 전에 설정한 카메라도 새 크기로 다시 적용된다 -----------------

void checkCameraKeptAcrossResize(Window& window, Renderer& renderer) {
    // size {0,0} 카메라는 1픽셀 = 1월드유닛이어야 한다. 크기는 setCamera 시점이 아니라 그리는 시점의 창 크기다.
    const Camera2D camera{.center = {0.f, 0.f}};
    renderer.setCamera(camera);
    check(renderer.camera().has_value() && renderer.camera()->center == camera.center,
          "setCamera한 카메라를 camera()로 돌려받는다");

    const bool resized = resize(window, renderer, {800u, 400u});
    check(resized, "창 크기를 800x400으로 바꿨다");
    if (!resized) {
        return;
    }

    // 다시 setCamera하지 않고 그대로 그린다. beginFrame이 새 크기로 다시 적용해야 한다.
    renderer.beginFrame(kClear);
    // 월드 x 380 = 화면 x 780. 새 크기(800)에서만 창 안에 들어온다.
    renderer.drawRect({{370.f, -10.f}, {20.f, 20.f}}, {.fill = kInk});
    const sf::Image image = capture(window);
    renderer.endFrame();

    check(renderer.camera().has_value() && renderer.camera()->center == camera.center,
          "리사이즈 뒤에도 카메라가 그대로 남아 있다");
    check(near(renderer.visibleWorldBounds().size, Vec2f{800.f, 400.f}),
          "리사이즈 뒤 size {0,0} 카메라: 보이는 영역 = 새 창 크기");
    check(isColor(image, 780, 200, kInk),
          "리사이즈 뒤 size {0,0} 카메라: 월드 1유닛 = 1픽셀로 그려진다");
    check(near(renderer.worldToScreen({380.f, 0.f}), Vec2f{780.f, 200.f}),
          "그린 위치와 worldToScreen이 일치한다");

    renderer.resetCamera();
    check(!renderer.camera().has_value(), "resetCamera() 뒤 camera() == nullopt");
}

// --- 뷰포트가 그리는 픽셀 ---------------------------------------------------

void checkLetterbox(Window& window, Renderer& renderer) {
    // 정사각형 설계 해상도를 800x400 창에: 400x400이 가운데 오고 좌우 200픽셀은 띠로 남는다.
    renderer.beginFrame(kClear);
    renderer.setCamera({.size = {100.f, 100.f}, .fit = CameraFit::Letterbox});
    renderer.drawRect(kEverywhere, {.fill = kInk});
    const sf::Image image = capture(window);
    renderer.endFrame();
    renderer.resetCamera();

    check(isColor(image, 199, 200, kClear) && isColor(image, 600, 200, kClear),
          "레터박스: 좌우 띠는 clearColor로 남는다");
    check(isColor(image, 200, 200, kInk) && isColor(image, 599, 200, kInk) &&
              isColor(image, 400, 0, kInk) && isColor(image, 400, 399, kInk),
          "레터박스: 안쪽 400x400은 경계 픽셀까지 그려진다");
}

void checkSplitScreen(Window& window, Renderer& renderer) {
    // 640 폭을 3등분. 비율을 SFML에 그대로 넘기면 426열이 비고, 모서리를 반올림하면 틈이 없다.
    const bool resized = resize(window, renderer, {640u, 480u});
    check(resized, "창 크기를 640x480으로 바꿨다");
    if (!resized) {
        return;
    }
    const float third = 1.f / 3.f;
    renderer.beginFrame(kClear);
    for (int i = 0; i < 3; ++i) {
        renderer.setCamera({.viewport = {static_cast<float>(i) * third, 0.f, third, 1.f}});
        renderer.drawRect(kEverywhere, {.fill = kInk});
    }
    const sf::Image image = capture(window);
    renderer.endFrame();
    renderer.resetCamera();

    bool noGap = true;
    for (unsigned int x = 0; x < 640; ++x) {
        noGap = noGap && isColor(image, x, 240, kInk);
    }
    check(noGap, "3등분 화면: 모든 열이 칠해진다 (틈 없음)");
}

void checkEmptyViewport(Window& window, Renderer& renderer) {
    // 크기 0인 뷰포트는 아무것도 그리지 않는다. 0으로 나눠 화면 전체에 번지거나 GL 오류를 내지 않는다.
    renderer.beginFrame(kClear);
    renderer.setCamera({.viewport = {0.5f, 0.5f, 0.f, 0.f}});
    renderer.drawRect(kEverywhere, {.fill = kInk});
    const sf::Image image = capture(window);
    renderer.endFrame();
    renderer.resetCamera();

    bool untouched = true;
    for (unsigned int y = 0; y < 480; y += 8) {
        for (unsigned int x = 0; x < 640; x += 8) {
            untouched = untouched && isColor(image, x, y, kClear);
        }
    }
    check(untouched, "크기 0 뷰포트: 아무 픽셀도 그리지 않는다");
}

} // namespace

int main() {
    if (env("MENO_DISPLAY_TESTS").empty() || env("MENO_DISPLAY_TESTS") == "0") {
        std::printf("SKIP 창과 GL 컨텍스트가 필요한 테스트. MENO_DISPLAY_TESTS = 1일 때만 돈다.\n");
        return kSkipReturnCode;
    }

    Window window{{320u, 180u}, "meno render resize test"};
    Renderer renderer{window};

    // 실제 게임처럼 창을 한 번 그린 뒤에 크기를 바꾼다.
    renderer.beginFrame(kClear);
    renderer.endFrame();

    // 크기를 못 바꾸는 환경(창 관리자가 막는 경우 등)이면 이후 검사는 의미가 없다.
    const bool resized = resize(window, renderer, {640u, 360u});
    check(resized, "창 크기를 320x180에서 640x360으로 바꿨다");
    if (resized) {
        checkScreenCoordinates(window, renderer);
        checkCameraKeptAcrossResize(window, renderer);
        checkLetterbox(window, renderer);
        checkSplitScreen(window, renderer);
        checkEmptyViewport(window, renderer);
    }

    return meno::test::report();
}
