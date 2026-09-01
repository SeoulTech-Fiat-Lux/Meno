#ifndef MENO_RENDERER_HPP
#define MENO_RENDERER_HPP

#include <meno/math/Camera2D.hpp>
#include <meno/math/Color.hpp>
#include <meno/graphics/DrawParams.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <memory>
#include <string_view>

namespace meno {

class Font;
class Texture;
class Window;

/// 2D 렌더 API.
///
/// 즉시 모드다. draw를 호출한 순서가 곧 그려지는 순서이고(나중 호출이 위),
/// 내부에 커맨드 버퍼나 정렬 단계가 없다. 레이어가 필요하면 호출 순서로
/// 표현한다 -- 배경 먼저, 캐릭터 나중.
///
/// 한 프레임의 형태:
///
///     renderer.beginFrame(colors::Black);
///     renderer.draw(playerTexture, {.position = {100.f, 50.f}});
///     renderer.endFrame();
///
/// 이 헤더는 SFML을 include하지 않는다. 사용자 빌드에 SFML 헤더 경로가
/// 없어도 컴파일된다.
class Renderer {
public:
    /// window는 Renderer보다 오래 살아야 한다. 소유하지 않는다.
    explicit Renderer(Window& window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) noexcept;
    Renderer& operator=(Renderer&&) noexcept;

    // --- 프레임 -----------------------------------------------------------

    /// 화면을 clearColor로 지우고 그리기를 시작한다.
    void beginFrame(Color clearColor = colors::Black);

    /// 이번 프레임에 쌓인 그리기를 화면에 표시한다.
    void endFrame();

    // --- 스프라이트 -------------------------------------------------------

    /// 텍스처를 position에 원본 크기로 그린다. 가장 짧은 형태.
    void draw(const Texture& texture, Vec2f position);

    /// 옵션을 지정해서 그린다. SpriteParams 참고.
    void draw(const Texture& texture, const SpriteParams& params);

    // --- 기본 도형 --------------------------------------------------------

    void drawRect(const Rectf& rect, const ShapeParams& params = {});
    void drawCircle(Vec2f center, float radius, const ShapeParams& params = {});
    void drawLine(Vec2f from, Vec2f to, Color color = colors::White, float thickness = 1.f);

    // --- 텍스트 -----------------------------------------------------------

    /// text는 UTF-8로 해석한다. 한글 문자열 리터럴이 그대로 들어간다.
    void drawText(const Font& font, std::string_view text, Vec2f position,
                  const TextParams& params = {});

    /// 그리지 않고 차지할 크기만 잰다. 가운데 정렬 등에 쓴다.
    [[nodiscard]] Vec2f measureText(const Font& font, std::string_view text,
                                    const TextParams& params = {}) const;

    // --- 카메라 -----------------------------------------------------------

    void setCamera(const Camera2D& camera);

    /// 1픽셀 = 1월드유닛인 기본 시점으로 되돌린다. UI를 그리기 전에 호출한다.
    void resetCamera();

    [[nodiscard]] Vec2f worldToScreen(Vec2f worldPoint) const;
    [[nodiscard]] Vec2f screenToWorld(Vec2f screenPoint) const;

    /// 현재 카메라로 보이는 월드 영역. 화면 밖 오브젝트를 건너뛸 때 쓴다.
    [[nodiscard]] Rectf visibleWorldBounds() const;

    // --- 정보 -------------------------------------------------------------

    [[nodiscard]] Vec2u framebufferSize() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace meno

#endif  // MENO_RENDERER_HPP