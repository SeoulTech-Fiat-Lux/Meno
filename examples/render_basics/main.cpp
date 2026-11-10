// 2D 렌더 API 전체를 한 화면에서 확인하는 예제.
//
// 에셋 파일 없이 그대로 돌아간다. 텍스처는 코드에서 만들고, 폰트는 시스템
// 폰트를 찾아보되 없으면 텍스트만 건너뛴다.
//
// 이 파일에는 SFML이 한 번도 등장하지 않는다. 그게 이 설계의 목표다.

#include <meno/core/Window.hpp>
#include <meno/core/Clock.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace {

/// 8x8 체커보드 RGBA 텍스처를 만든다. 에셋 없이 스프라이트를 시험하기 위한 것.
meno::Texture makeCheckerTexture() {
    constexpr unsigned int kSize = 8;
    std::vector<std::uint8_t> pixels(kSize * kSize * 4);

    for (unsigned int y = 0; y < kSize; ++y) {
        for (unsigned int x = 0; x < kSize; ++x) {
            const bool light = ((x / 2) + (y / 2)) % 2 == 0;
            const std::size_t index = (y * kSize + x) * 4;
            pixels[index + 0] = light ? 240 : 70;
            pixels[index + 1] = light ? 200 : 60;
            pixels[index + 2] = light ? 120 : 120;
            pixels[index + 3] = 255;
        }
    }

    auto texture = meno::Texture::fromPixels({kSize, kSize}, pixels.data());
    if (!texture) {
        // 크기와 버퍼가 상수라 실패할 일은 없지만, optional을 검사 없이
        // 역참조하는 예시를 남기지 않는다.
        return meno::Texture{};
    }

    // 픽셀아트라 보간을 끈다. 켜면 확대했을 때 뿌옇게 뭉개진다.
    texture->setSmooth(false);
    return std::move(*texture);
}

/// 한글 글리프가 있는 시스템 폰트를 찾아본다. 없으면 nullopt.
std::optional<meno::Font> findSystemFont() {
    const std::array<const char*, 4> candidates{
        "C:/Windows/Fonts/malgun.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/nanum/NanumGothic.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };

    for (const char* path : candidates) {
        if (std::filesystem::exists(path)) {
            if (auto font = meno::Font::loadFromFile(path)) {
                return font;
            }
        }
    }
    return std::nullopt;
}

/// 월드에 놓인 것들을 그린다. 메인 카메라와 미니맵이 같은 월드를 서로 다른 카메라로 비춘다.
void drawWorld(meno::Renderer& renderer, const meno::Texture& checker, double elapsed) {
    // 기준선. 원점이 어디인지 보여준다.
    renderer.drawLine({-400.f, 0.f}, {400.f, 0.f}, meno::colors::Gray, 1.f);
    renderer.drawLine({0.f, -220.f}, {0.f, 220.f}, meno::colors::Gray, 1.f);

    // 도형: 채우기 + 외곽선
    renderer.drawRect({{-320.f, -160.f}, {120.f, 80.f}},
                      {.fill = meno::Color::fromRgb(0x4C6EF5),
                       .outline = meno::colors::White,
                       .outlineThickness = 2.f});

    renderer.drawCircle({-260.f, 120.f}, 45.f,
                        {.fill = meno::Color::fromRgb(0x51CF66),
                         .outline = meno::colors::Black,
                         .outlineThickness = 3.f});

    // 스프라이트: 중심 회전 + 확대
    // origin을 텍스처 크기의 절반으로 두면 제자리에서 돈다.
    const auto textureSize = static_cast<meno::Vec2f>(checker.size());
    renderer.draw(checker, {.position = {0.f, 0.f},
                            .origin = textureSize / 2.f,
                            .scale = {10.f, 10.f},
                            .rotation = static_cast<float>(elapsed * 45.0)});

    // 같은 텍스처를 tint와 반투명으로 재사용
    renderer.draw(checker, {.position = {200.f, -80.f},
                            .origin = textureSize / 2.f,
                            .scale = {6.f, 6.f},
                            .tint = meno::colors::Cyan.withAlpha(160)});

    // 좌우 반전
    renderer.draw(checker, {.position = {200.f, 100.f},
                            .origin = textureSize / 2.f,
                            .scale = {6.f, 6.f},
                            .flipX = true});
}

} // namespace

int main() {
    meno::Window window{{960u, 540u}, "meno - 2D 렌더 기초"};
    meno::Renderer renderer{window};

    const meno::Texture checker = makeCheckerTexture();
    const std::optional<meno::Font> font = findSystemFont();

    double elapsed = 0.0;
    meno::Clock clock;
    clock.setFramerateLimit(60);

    while (window.isOpen()) {
        elapsed += clock.restart();

        window.pollEvents();

        // 창 크기를 바꿔도 따로 할 일이 없다. beginFrame이 지금 창 크기로 카메라를 다시 적용한다.
        renderer.beginFrame(meno::Color::fromRgb(0x1E2430));

        // --- 월드: 카메라가 적용되는 영역 --------------------------------
        // size를 정하지 않았으므로 1월드유닛 = 1픽셀이다. 창을 키우면 월드가 더 넓게 보인다.
        // 카메라가 시계 방향으로 돌면 화면 속 월드는 반시계 방향으로 돈다.
        const meno::Camera2D camera{
            .center = {0.f, 0.f},
            .zoom = 1.f + 0.15f * static_cast<float>(std::sin(elapsed * 0.7)),
            .rotation = 12.f * static_cast<float>(std::sin(elapsed * 0.4))};
        renderer.setCamera(camera);
        drawWorld(renderer, checker, elapsed);

        // --- 미니맵: 화면 우상단의 작은 뷰포트 ----------------------------
        // 월드 1920x1080을 16:9 고정 비율로 담는다. 창의 종횡비가 달라도 Letterbox가 비율을 지키고 남는 부분은 비운다.
        const meno::Camera2D minimap{.center = {0.f, 0.f},
                                     .size = {1920.f, 1080.f},
                                     .viewport = {0.70f, 0.03f, 0.27f, 0.27f},
                                     .fit = meno::CameraFit::Letterbox};
        renderer.setCamera(minimap);
        // 뷰포트 밖으로는 그려지지 않는다. 아주 큰 사각형으로 미니맵 배경만 칠할 수 있다.
        renderer.drawRect({{-5000.f, -5000.f}, {10000.f, 10000.f}},
                          {.fill = meno::Color::fromRgb(0x10141C)});
        drawWorld(renderer, checker, elapsed);
        // 메인 카메라가 지금 보고 있는 영역. 회전하면 그것을 감싸는 사각형이 된다.
        // math/Camera2D.hpp의 함수는 Renderer 없이도 같은 규칙으로 계산한다.
        renderer.drawRect(meno::visibleWorldBounds(camera, renderer.framebufferSize()),
                          {.fill = meno::colors::Transparent,
                           .outline = meno::colors::Yellow,
                           .outlineThickness = 16.f});

        // --- UI: 카메라를 되돌려 화면 좌표로 그린다 ----------------------
        // 화면 좌표는 창 크기를 따라간다. 창을 키워도 UI가 늘어나지 않는다.
        renderer.resetCamera();

        // 미니맵 테두리. viewportPixels는 미니맵이 실제로 차지한 픽셀(레터박스 반영)을 돌려준다.
        renderer.drawRect(meno::viewportPixels(minimap, renderer.framebufferSize()),
                          {.fill = meno::colors::Transparent,
                           .outline = meno::colors::White,
                           .outlineThickness = 1.f});

        if (font) {
            const meno::TextParams title{.characterSize = 20,
                                          .color = meno::colors::White,
                                          .outlineThickness = 2.f,
                                          .outlineColor = meno::colors::Black};
            renderer.drawText(*font, "meno 2D 렌더 API", {16.f, 12.f}, title);

            // measureTextBounds()로 글자가 실제 차지하는 영역을 계산하여, 화면 오른쪽 아래에 붙인다. (우하단 정렬)
            // 글자 잉크의 우하단이 화면 우하단으로부터 16픽셀 안쪽으로 떨어지도록 위치를 계산한다.
            // {0, 0}이 아니기 때문에, 크기만 활용하는 mearueText()로는 아래쪽이 몇 픽셀 어긋난다.
            const meno::TextParams hint{.characterSize = 14,
                                         .color = meno::colors::Gray};
            const char* hintText = "카메라가 자동으로 줌·회전합니다. 창 크기를 바꿔 보세요";
            const meno::Rectf hintBounds = renderer.measureTextBounds(*font, hintText, hint);
            const auto screen = static_cast<meno::Vec2f>(renderer.framebufferSize());
            const meno::Vec2f inkBottomRight{hintBounds.right(), hintBounds.bottom()};
            renderer.drawText(*font, hintText,
                              screen - meno::Vec2f{16.f, 16.f} - inkBottomRight, hint);
        }

        renderer.endFrame();
    }

    return 0;
}
