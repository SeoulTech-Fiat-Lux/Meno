// 카메라 수학(math/Camera2D.hpp)과 SFML이 같은 규칙을 따르는지 대조한다.
//
// Renderer는 그릴 때 backend::toSfView로 만든 sf::View를 SFML에 넘기고,
// worldToScreen 같은 좌표 계산은 SFML을 거치지 않고 math 함수로 한다.
// 둘이 한 픽셀이라도 어긋나면 마우스로 고른 물체와 화면에 보이는 물체가 달라진다.

// 창 없이 크기만 가진 가짜 렌더 타깃으로 SFML의 좌표 변환 함수를 직접 부른다.
// sf::RenderTarget의 좌표 변환은 GL 컨텍스트를 쓰지 않으므로 디스플레이 없이 돈다.

#include <meno/math/Camera2D.hpp>

#include "sfml_backend/Convert.hpp"

#include "check.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>

using namespace meno;

namespace {

using meno::test::check;

// 크기만 가진 렌더 타깃. getViewport, mapCoordsToPixel, mapPixelToCoords는 getSize()만 쓴다.
class FakeTarget final : public sf::RenderTarget {
public:
    explicit FakeTarget(Vec2u size) : size_(backend::toSf(size)) {}
    [[nodiscard]] sf::Vector2u getSize() const override { return size_; }

private:
    sf::Vector2u size_;
};

// 재현 가능한 의사 난수. 실패하면 같은 시드로 다시 돌려 볼 수 있어야 한다.
class Random {
public:
    explicit Random(std::uint32_t seed) : state_(seed * 2654435761u + 1u) {}

    float uniform(float low, float high) {
        state_ = state_ * 1664525u + 1013904223u;
        return low + (high - low) * static_cast<float>(state_ >> 8) / static_cast<float>(1u << 24);
    }
    bool chance(float probability) { return uniform(0.f, 1.f) < probability; }

private:
    std::uint32_t state_;
};

// SFML의 mapCoordsToPixel은 결과를 정수로 자른다(Vector2i). 같은 계산을 자르기 직전까지만 따라 한다.
// (SFML/Graphics/RenderTarget.cpp의 mapCoordsToPixel과 같은 식)
Vec2f sfmlWorldToScreen(const FakeTarget& target, const sf::View& view, Vec2f world) {
    const sf::Vector2f normalized = view.getTransform().transformPoint(backend::toSf(world));
    const sf::FloatRect viewport{target.getViewport(view)};
    return Vec2f{(normalized.x + 1.f) / 2.f * viewport.size.x + viewport.position.x,
                 (1.f - normalized.y) / 2.f * viewport.size.y + viewport.position.y};
}

float distance(Vec2f a, Vec2f b) {
    return (a - b).length();
}

struct Sample {
    Camera2D camera;
    Vec2u framebuffer;
};

Sample makeSample(std::uint32_t seed) {
    Random random{seed};
    Sample sample;
    sample.framebuffer = {static_cast<unsigned int>(random.uniform(64.f, 2048.f)),
                          static_cast<unsigned int>(random.uniform(64.f, 2048.f))};

    Camera2D& camera = sample.camera;
    camera.center = {random.uniform(-2000.f, 2000.f), random.uniform(-2000.f, 2000.f)};
    if (random.chance(0.5f)) {
        camera.size = {random.uniform(100.f, 4000.f), random.uniform(100.f, 4000.f)};
    }
    camera.zoom = random.uniform(0.25f, 4.f);
    camera.rotation = random.uniform(-10000.f, 10000.f);
    if (random.chance(0.6f)) {
        // 그릴 픽셀이 있도록 최소 크기를 둔다. 빈 뷰포트는 SFML이 0으로 나누므로 대조할 수 없다.
        const float width = random.uniform(0.1f, 1.f);
        const float height = random.uniform(0.1f, 1.f);
        camera.viewport = {random.uniform(0.f, 1.f - width), random.uniform(0.f, 1.f - height),
                           width, height};
    }
    camera.fit = random.chance(0.5f) ? CameraFit::Letterbox : CameraFit::Stretch;
    return sample;
}

} // namespace

int main() {
    // --- 무작위 카메라 대조 ---------------------------------------------------
    constexpr std::uint32_t kSeeds = 3000;
    std::uint32_t viewportMismatches = 0;
    std::uint32_t transformMismatches = 0;
    std::uint32_t boundsMismatches = 0;
    float worstRatio = 0.f; // 오차 / 허용치의 최댓값. 1을 넘으면 실패다.

    for (std::uint32_t seed = 0; seed < kSeeds; ++seed) {
        const auto [camera, framebuffer] = makeSample(seed);
        const FakeTarget target{framebuffer};
        const sf::View view = backend::toSfView(camera, framebuffer);

        // 1. 뷰포트 픽셀: SFML이 반올림한 정수 사각형이 viewportPixels와 정확히 같아야 한다.
        const sf::IntRect sfmlPixels = target.getViewport(view);
        const Rectf pixels = viewportPixels(camera, framebuffer);
        if (static_cast<float>(sfmlPixels.position.x) != pixels.left() ||
            static_cast<float>(sfmlPixels.position.y) != pixels.top() ||
            static_cast<float>(sfmlPixels.size.x) != pixels.size.x ||
            static_cast<float>(sfmlPixels.size.y) != pixels.size.y) {
            if (viewportMismatches++ == 0) {
                std::printf(
                    "   seed %u: SFML viewport {%d,%d,%d,%d} != viewportPixels {%g,%g,%g,%g}\n",
                    seed, sfmlPixels.position.x, sfmlPixels.position.y, sfmlPixels.size.x,
                    sfmlPixels.size.y, pixels.left(), pixels.top(), pixels.size.x, pixels.size.y);
            }
            continue;
        }
        if (pixels.size.x <= 0.f || pixels.size.y <= 0.f) {
            continue;
        }

        // 허용치: 픽셀 단위. float의 상대 오차는 좌표의 크기에 비례하므로, 월드 좌표의 크기와
        // 월드 1유닛이 차지하는 픽셀 수(배율)를 곱해 둔다. 규칙이 어긋나면 수 픽셀 이상 틀리므로 구분된다.
        const Vec2f viewSize = viewRect(camera, framebuffer).size;
        const float pixelsPerUnit =
            std::max(pixels.size.x / viewSize.x, pixels.size.y / viewSize.y);

        // 2. 좌표 변환: 뷰포트 안팎의 정수 픽셀에서 양방향을 대조한다.
        Random random{seed + 7919u};
        bool transformOk = true;
        for (int i = 0; i < 16; ++i) {
            const sf::Vector2i pixel{
                static_cast<int>(random.uniform(pixels.left() - 50.f, pixels.right() + 50.f)),
                static_cast<int>(random.uniform(pixels.top() - 50.f, pixels.bottom() + 50.f))};
            const Vec2f screen{static_cast<float>(pixel.x), static_cast<float>(pixel.y)};

            const Vec2f world = screenToWorld(camera, framebuffer, screen);
            const Vec2f sfmlWorld = backend::fromSf(target.mapPixelToCoords(pixel, view));
            const float magnitude =
                std::abs(camera.center.x) + std::abs(camera.center.y) + viewSize.x + viewSize.y;
            const float tolerance = 0.01f + 16.f * FLT_EPSILON * magnitude * pixelsPerUnit;

            const float screenToWorldError = distance(world, sfmlWorld) * pixelsPerUnit;
            const float worldToScreenError = distance(worldToScreen(camera, framebuffer, world),
                                                      sfmlWorldToScreen(target, view, world));
            worstRatio = std::max(
                {worstRatio, screenToWorldError / tolerance, worldToScreenError / tolerance});
            transformOk =
                transformOk && screenToWorldError <= tolerance && worldToScreenError <= tolerance;
        }
        if (!transformOk && transformMismatches++ == 0) {
            std::printf("   seed %u: 좌표 변환이 SFML과 어긋난다\n", seed);
        }

        // 3. 가시 영역: NDC 사각형 [-1,1]^2를 SFML의 역변환으로 월드에 옮긴 외접 사각형과 같아야 한다.
        const sf::FloatRect sfmlBounds =
            view.getInverseTransform().transformRect({{-1.f, -1.f}, {2.f, 2.f}});
        const Rectf bounds = visibleWorldBounds(camera, framebuffer);
        const float boundsTolerance =
            0.01f + 16.f * FLT_EPSILON *
                        (std::abs(camera.center.x) + std::abs(camera.center.y) + bounds.size.x +
                         bounds.size.y);
        if ((distance(backend::fromSf(sfmlBounds.position), bounds.position) > boundsTolerance ||
             distance(backend::fromSf(sfmlBounds.size), bounds.size) > boundsTolerance) &&
            boundsMismatches++ == 0) {
            std::printf("   seed %u: visibleWorldBounds가 SFML과 어긋난다\n", seed);
        }
    }

    std::printf("   (시드 %u개, 최대 오차 = 허용치의 %.0f%%)\n", kSeeds, worstRatio * 100.f);
    check(viewportMismatches == 0, "SFML이 반올림한 뷰포트 픽셀 == viewportPixels (모든 시드)");
    check(transformMismatches == 0,
          "worldToScreen / screenToWorld == SFML의 좌표 변환 (모든 시드)");
    check(boundsMismatches == 0, "visibleWorldBounds == SFML 뷰의 외접 사각형 (모든 시드)");

    // --- 고정 사례 ------------------------------------------------------------

    // 3등분 화면. SFML에 비율을 그대로 넘기면 위치와 크기를 따로 반올림해서 426열이 빈다.
    // toSfView는 모서리를 반올림한 픽셀을 넘기므로 SFML 쪽에서도 틈이 없다.
    {
        constexpr Vec2u kOdd{640u, 480u};
        const FakeTarget target{kOdd};
        const float third = 1.f / 3.f;
        int previousRight = 0;
        bool contiguous = true;
        for (int i = 0; i < 3; ++i) {
            const Camera2D camera{.viewport = {static_cast<float>(i) * third, 0.f, third, 1.f}};
            const sf::IntRect pixels = target.getViewport(backend::toSfView(camera, kOdd));
            contiguous = contiguous && pixels.position.x == previousRight;
            previousRight = pixels.position.x + pixels.size.x;
        }
        check(contiguous && previousRight == 640,
              "3등분 화면: SFML이 실제로 쓰는 뷰포트에도 틈이 없다");
    }

    // 아주 큰 각도. 360으로 접지 않고 넘기면 SFML이 라디안으로 바꾸며 정밀도를 잃어 몇 픽셀씩 어긋난다.
    {
        constexpr Vec2u kScreen{1280u, 720u};
        const FakeTarget target{kScreen};
        const Camera2D camera{.center = {300.f, -200.f}, .rotation = 10000037.f};
        const sf::View view = backend::toSfView(camera, kScreen);
        const Vec2f corner = screenToWorld(camera, kScreen, {0.f, 0.f});
        check(distance(sfmlWorldToScreen(target, view, corner), Vec2f{0.f, 0.f}) < 0.01f,
              "큰 각도(1e7도): SFML이 그리는 위치와 좌표 계산이 일치한다");
    }

    // 창 크기가 0이면 비율로 바꿀 수 없으므로 뷰포트를 건드리지 않는다. (NaN 뷰포트 방지)
    {
        const sf::FloatRect viewport = backend::toSfView(Camera2D{}, {0u, 0u}).getViewport();
        check(viewport.position == sf::Vector2f{0.f, 0.f} &&
                  viewport.size == sf::Vector2f{1.f, 1.f},
              "창 크기 {0,0}: 뷰포트는 기본값 그대로 (NaN 없음)");
    }

    return meno::test::report();
}
