// 2D 렌더 API의 주요 기능을 한 화면에서 확인하는 예제.
//
// Game 없이 Window, Renderer, Clock으로 루프를 직접 구현한다. 에셋 파일 없이 실행되며, 텍스처는
// 코드로 만들고 글자는 시스템 폰트로 그린다. 폰트가 없으면 글자만 그리지 않는다. 창을 닫으면
// 종료한다.
//
// 이 파일에는 SFML이 한 번도 나오지 않는다. 사용자 코드가 SFML을 직접 다루지 않게 하는 것이
// meno의 설계 목표다.

#include <meno/core/Clock.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>
#include <meno/window/Window.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace {

// 8×8 체커보드 RGBA 텍스처를 만든다. 에셋 없이 스프라이트를 시험하는 데 쓴다.
meno::Texture makeCheckerTexture() {
    constexpr unsigned int kSize = 8;
    std::vector<std::uint8_t> pixels(std::size_t{kSize} * kSize * 4);

    for (unsigned int y = 0; y < kSize; ++y) {
        for (unsigned int x = 0; x < kSize; ++x) {
            const bool light = ((x / 2) + (y / 2)) % 2 == 0;
            const std::size_t index = (std::size_t{y} * kSize + x) * 4;
            pixels[index + 0] = light ? 240 : 70;
            pixels[index + 1] = light ? 200 : 60;
            pixels[index + 2] = 120; // 파랑 성분은 밝은 칸과 어두운 칸이 같다
            pixels[index + 3] = 255;
        }
    }

    auto texture = meno::Texture::fromPixels({kSize, kSize}, pixels.data());
    if (!texture) {
        // 크기와 버퍼가 상수이므로 실패할 일은 없지만, optional을 검사하지 않고 역참조하는 예를
        // 남기지 않는다.
        return meno::Texture{};
    }

    // 픽셀 아트이므로 보간을 끈다. 보간을 켜면 확대했을 때 흐려진다.
    texture->setSmooth(false);
    return std::move(*texture);
}

// 후보 경로에서 시스템 폰트를 찾고, 모두 없으면 nullopt를 돌려준다. 맑은 고딕과 나눔고딕 외의
// 후보에는 한글 글리프가 없다.
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

// 월드에 놓인 것을 그린다. 메인 화면과 미니맵이 모두 이 함수를 호출하며, 카메라만 서로 다르다.
void drawWorld(meno::Renderer& renderer, const meno::Texture& checker, double elapsed) {
    // 기준선. 원점의 위치를 보여 준다.
    renderer.drawLine({-400.f, 0.f}, {400.f, 0.f}, {.color = meno::colors::Gray});
    renderer.drawLine({0.f, -220.f}, {0.f, 220.f}, {.color = meno::colors::Gray});

    // 도형: 채우기와 외곽선
    renderer.drawRect({{-320.f, -160.f}, {120.f, 80.f}}, {.fill = meno::Color::fromRgb(0x4C6EF5),
                                                          .outline = meno::colors::White,
                                                          .outlineThickness = 2.f});

    renderer.drawCircle({-260.f, 120.f}, 45.f,
                        {.fill = meno::Color::fromRgb(0x51CF66),
                         .outline = meno::colors::Black,
                         .outlineThickness = 3.f});

    // 스프라이트: 중심 회전과 확대
    // origin을 텍스처 크기의 절반으로 두면 텍스처가 제자리에서 회전한다.
    const auto textureSize = static_cast<meno::Vec2f>(checker.size());
    renderer.draw(checker, {.position = {0.f, 0.f},
                            .origin = textureSize / 2.f,
                            .scale = {10.f, 10.f},
                            .rotation = static_cast<float>(elapsed * 45.0)});

    // 같은 텍스처에 색(tint)을 곱하고 반투명하게 그린다
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

        // 창 크기가 바뀌어도 따로 처리할 필요가 없다. beginFrame()이 현재 창 크기에 맞춰 카메라를
        // 다시 적용한다.
        renderer.beginFrame(meno::Color::fromRgb(0x1E2430));

        // --- 월드: 카메라가 적용되는 영역 --------------------------------
        // size를 정하지 않았으므로 월드 1단위가 1픽셀이다. 창을 키우면 월드가 더 넓게 보인다.
        // 카메라가 시계 방향으로 회전하면 화면 속 월드는 반시계 방향으로 회전한다.
        const meno::Camera2D camera{.center = {0.f, 0.f},
                                    .zoom =
                                        1.f + 0.15f * static_cast<float>(std::sin(elapsed * 0.7)),
                                    .rotation = 12.f * static_cast<float>(std::sin(elapsed * 0.4))};
        renderer.setCamera(camera);
        drawWorld(renderer, checker, elapsed);

        // --- 미니맵: 화면 오른쪽 위의 작은 뷰포트 ------------------------
        // 월드의 1920×1080 영역을 16:9 비율 그대로 담는다. 창의 종횡비가 달라도 Letterbox가 비율을
        // 지키고 남는 부분은 비워 둔다.
        const meno::Camera2D minimap{.center = {0.f, 0.f},
                                     .size = {1920.f, 1080.f},
                                     .viewport = {0.70f, 0.03f, 0.27f, 0.27f},
                                     .fit = meno::CameraFit::Letterbox};
        renderer.setCamera(minimap);
        // 뷰포트 밖에는 그려지지 않으므로, 아주 큰 사각형을 그려 미니맵 배경만 칠할 수 있다.
        renderer.drawRect({{-5000.f, -5000.f}, {10000.f, 10000.f}},
                          {.fill = meno::Color::fromRgb(0x10141C)});
        drawWorld(renderer, checker, elapsed);
        // 메인 카메라가 지금 비추는 영역이다. 카메라가 회전하면 그 영역을 감싸는 사각형이 된다.
        // math/Camera2D.hpp의 함수는 Renderer 없이도 같은 규칙으로 계산한다.
        renderer.drawRect(meno::visibleWorldBounds(camera, renderer.framebufferSize()),
                          {.fill = meno::colors::Transparent,
                           .outline = meno::colors::Yellow,
                           .outlineThickness = 16.f});

        // --- UI: 카메라를 되돌려 화면 좌표로 그린다 ----------------------
        // 화면 좌표는 창의 픽셀 좌표이므로, 창을 키워도 UI가 늘어나지 않는다.
        renderer.resetCamera();

        // 미니맵 테두리. viewportPixels()로 레터박스를 반영한, 미니맵의 실제 픽셀 영역을 얻는다.
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

            // 안내 문구를 화면 오른쪽 아래에 붙인다. measureTextBounds()로 글자 잉크가 실제로
            // 차지하는 영역을 구해, 잉크의 오른쪽 아래 끝이 화면 모서리에서 16픽셀 안쪽에 오도록
            // 위치를 정한다. 잉크 영역은 {0, 0}에서 시작하지 않으므로, 크기만 돌려주는
            // measureText()로 계산하면 아래쪽이 몇 픽셀 어긋난다.
            const meno::TextParams hint{.characterSize = 14, .color = meno::colors::Gray};
            const char* hintText = "카메라가 자동으로 줌·회전합니다. 창 크기를 바꿔 보세요";
            const meno::Rectf hintBounds = renderer.measureTextBounds(*font, hintText, hint);
            const auto screen = static_cast<meno::Vec2f>(renderer.framebufferSize());
            const meno::Vec2f inkBottomRight{hintBounds.right(), hintBounds.bottom()};
            renderer.drawText(*font, hintText, screen - meno::Vec2f{16.f, 16.f} - inkBottomRight,
                              hint);
        }

        renderer.endFrame();
    }

    return 0;
}
