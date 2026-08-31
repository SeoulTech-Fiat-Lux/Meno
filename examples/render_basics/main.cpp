// 2D 렌더 API 전체를 한 화면에서 확인하는 예제.
//
// 에셋 파일 없이 그대로 돌아간다. 텍스처는 코드에서 만들고, 폰트는 시스템
// 폰트를 찾아보되 없으면 텍스트만 건너뛴다.
//
// 이 파일에는 SFML이 한 번도 등장하지 않는다. 그게 이 설계의 목표다.

#include <meno/core/Window.hpp>
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

        renderer.beginFrame(meno::Color::fromHex(0x1E2430FF));

        // --- 월드: 카메라가 적용되는 영역 --------------------------------
        meno::Camera2D camera;
        camera.center = {0.f, 0.f};
        camera.zoom =
            1.f + 0.15f * static_cast<float>(std::sin(elapsed * 0.7));
        renderer.setCamera(camera);

        // 기준선. 원점이 어디인지 보여준다.
        renderer.drawLine({-400.f, 0.f}, {400.f, 0.f}, meno::colors::Gray, 1.f);
        renderer.drawLine({0.f, -220.f}, {0.f, 220.f}, meno::colors::Gray, 1.f);

        // 도형: 채우기 + 외곽선
        renderer.drawRect({{-320.f, -160.f}, {120.f, 80.f}},
                          {.fill = meno::Color::fromHex(0x4C6EF5FF),
                           .outline = meno::colors::White,
                           .outlineThickness = 2.f});

        renderer.drawCircle({-260.f, 120.f}, 45.f,
                            {.fill = meno::Color::fromHex(0x51CF66FF),
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

        // --- UI: 카메라를 되돌려 화면 좌표로 그린다 ----------------------
        renderer.resetCamera();

        if (font) {
            const meno::TextParams title{.characterSize = 20,
                                          .color = meno::colors::White,
                                          .outlineThickness = 2.f,
                                          .outlineColor = meno::colors::Black};
            renderer.drawText(*font, "meno 2D 렌더 API", {16.f, 12.f}, title);

            // measureText로 우측 정렬
            const meno::TextParams hint{.characterSize = 14,
                                         .color = meno::colors::Gray};
            const char* hintText = "카메라 줌이 자동으로 변합니다";
            const meno::Vec2f hintSize = renderer.measureText(*font, hintText, hint);
            const auto screen = static_cast<meno::Vec2f>(renderer.framebufferSize());
            renderer.drawText(*font, hintText,
                              {screen.x - hintSize.x - 16.f, screen.y - hintSize.y - 16.f}, hint);
        }

        renderer.endFrame();
    }

    return 0;
}
