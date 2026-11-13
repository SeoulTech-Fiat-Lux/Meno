#ifndef MENO_RENDERER_HPP
#define MENO_RENDERER_HPP

#include <meno/graphics/DrawParams.hpp>
#include <meno/math/Camera2D.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <initializer_list>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace meno {

/// \addtogroup graphics
/// \{

class Font;
class Texture;
class Window;

// 즉시 모드다. 내부에 커맨드 버퍼나 정렬 단계가 없고, draw 호출이 바로 렌더 타깃에 그린다.
/// 창에 텍스처, 도형, 텍스트를 그립니다.
///
/// 부른 순서대로 그리므로 나중에 그린 것이 위에 옵니다. 레이어는 호출 순서로 정합니다(배경 먼저,
/// 캐릭터 나중). 카메라를 정하지 않으면 화면 좌표(창 좌상단이 원점, 1픽셀 = 1월드유닛)로
/// 그립니다.
///
/// 이 Renderer를 만든 Window가 Renderer보다 오래 살아야 합니다. 텍스처와 폰트는 그리는 호출
/// 동안만 살아 있으면 됩니다.
///
/// 한 프레임의 형태:
///
/// \code{.cpp}
/// window.pollEvents();
/// renderer.beginFrame(meno::colors::Black);
/// renderer.draw(playerTexture, {.position = {100.f, 50.f}});
/// renderer.endFrame();
/// \endcode
///
/// \see Window, Camera2D, SpriteParams, ShapeParams, TextParams
class Renderer {
public:
    // 생성 시점에 window의 렌더 타깃을 붙잡는다. Window는 이동할 수 없으므로 붙잡은 타깃은
    // 끝까지 유효하다.
    /// window에 그리는 Renderer를 만듭니다. window를 소유하지 않으므로 window가 더 오래 살아야
    /// 합니다.
    explicit Renderer(Window& window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /// 화면을 clearColor로 지우고 그리기를 시작합니다. 프레임마다 처음에 부릅니다.
    ///
    /// 카메라를 지금의 창 크기에 맞춰 다시 적용하므로 창 크기가 바뀌어도 따로 할 일이 없습니다.
    /// 바뀐 크기는 Window::pollEvents()가 반영하므로 pollEvents()를 먼저 부릅니다.
    void beginFrame(Color clearColor = colors::Black);

    /// 이번 프레임에 그린 것을 화면에 표시합니다. 프레임마다 마지막에 부릅니다.
    void endFrame();

    /// 텍스처를 position에 원본 크기로 그립니다. position에는 텍스처의 좌상단이 놓입니다.
    /// 로드하지 않았거나 이동된 텍스처는 그리지 않습니다.
    void draw(const Texture& texture, Vec2f position);

    /// SpriteParams의 옵션으로 텍스처를 그립니다. 로드하지 않았거나 이동된 텍스처는 그리지
    /// 않습니다.
    void draw(const Texture& texture, const SpriteParams& params);

    /// 사각형을 그립니다.
    ///
    /// rect.position에는 params.origin이 놓입니다. origin은 사각형의 좌상단에서 잰 점이므로,
    /// 기본값 {0, 0}이면 rect.position이 좌상단이고, rect.size의 절반이면 rect.position이
    /// 중심입니다.
    void drawRect(const Rectf& rect, const ShapeParams& params = {});

    /// center를 중심으로 원을 그립니다. radius가 양의 유한수가 아니면 그리지 않습니다.
    ///
    /// params.origin은 원의 중심에서 잰 점으로, 그 점이 center에 놓이고 회전 중심이 됩니다.
    void drawCircle(Vec2f center, float radius, const ShapeParams& params = {});
    /// from에서 to까지 두께 있는 선을 그립니다. 두께는 선 양쪽으로 반씩 퍼집니다.
    /// 길이나 두께가 양의 유한수가 아니면(0, 음수, NaN, 무한대) 그리지 않습니다.
    void drawLine(Vec2f from, Vec2f to, const LineParams& params = {});

    // SFML이 바운딩 박스 중심에서 부채를 펴는 식으로 면을 채우므로 볼록한 다각형만 보장한다.
    /// 점들을 순서대로 이은 다각형을 그립니다. 마지막 점은 첫 점과 이어집니다.
    ///
    /// points는 position을 원점으로 한 로컬 좌표입니다. params.origin은 position에 놓일 로컬 점이자
    /// 회전 중심이며, 기본값 {0, 0}이면 position을 축으로 돕니다. 점이 3개 미만이면 그리지
    /// 않습니다. points는 이 호출 안에서만 읽으므로 호출이 끝나면 버려도 됩니다.
    ///
    /// 면 채우기는 볼록한 다각형만 올바르게 그립니다. 외곽선만 그리면 오목해도 됩니다.
    void drawPolygon(Vec2f position, std::span<const Vec2f> points, const ShapeParams& params = {});

    // C++20의 std::span은 {a, b, c} 목록을 받지 못하므로 오버로드를 따로 둔다.
    /// 점을 중괄호 목록으로 바로 넘기는 drawPolygon()입니다. 나머지는 span을 받는 쪽과 같습니다.
    ///
    /// \code{.cpp}
    /// renderer.drawPolygon(pos, {{0.f, -12.f}, {9.f, 8.f}, {-9.f, 8.f}}, {.fill = colors::Red});
    /// \endcode
    void drawPolygon(Vec2f position, std::initializer_list<Vec2f> points,
                     const ShapeParams& params = {});

    /// center를 중심으로 부채꼴을 그립니다. 쿨다운 표시처럼 일부만 채운 원에 씁니다.
    ///
    /// 각도는 도 단위이며 오른쪽(+x)이 0도이고 시계 방향으로 커집니다. startDeg에서 시작해
    /// sweepDeg만큼 펼치고, sweepDeg가 음수면 반시계 방향입니다. |sweepDeg|가 360 이상이면
    /// drawCircle()과 같습니다. radius가 양의 유한수가 아니거나 sweepDeg가 0이나 NaN이면 그리지
    /// 않습니다. params.origin과 rotation은 drawCircle()처럼 중심을 기준으로 합니다.
    ///
    /// \code{.cpp}
    /// // 위쪽(-90도)에서 시작해 시계 방향으로 남은 비율만큼 채웁니다.
    /// renderer.drawSector(center, 20.f, -90.f, 360.f * remaining, {.fill = colors::Gray});
    /// \endcode
    void drawSector(Vec2f center, float radius, float startDeg, float sweepDeg,
                    const ShapeParams& params = {});

    /// text를 그립니다. position에는 params.origin이 놓이며, origin은 텍스트 상자의 좌상단에서
    /// 잽니다. 로드하지 않았거나 이동된 Font면 그리지 않습니다.
    ///
    /// text는 UTF-8로 해석합니다. MSVC에서는 사용자 프로젝트도 /utf-8로 컴파일해야 한글 문자열
    /// 리터럴이 깨지지 않습니다.
    void drawText(const Font& font, std::string_view text, Vec2f position,
                  const TextParams& params = {});

    /// 그리지 않고 차지할 크기만 잽니다. 같은 params로 drawText()한 결과와 크기가 같습니다.
    ///
    /// 글자의 실제 잉크는 drawText()의 position에서 바로 시작하지 않습니다. 윗부분 여백과
    /// 외곽선만큼 어긋나 있어서, 크기만으로 정렬하면 몇 픽셀 틀어집니다. 픽셀 단위로 맞추려면
    /// measureTextBounds()를 씁니다. 로드하지 않았거나 이동된 Font면 {0, 0}입니다.
    [[nodiscard]] Vec2f measureText(const Font& font, std::string_view text,
                                    const TextParams& params = {}) const;

    /// drawText(font, text, {0, 0}, params)로 그렸을 때 실제 잉크가 차지하는 영역입니다.
    /// params의 origin과 rotation을 적용하기 전의 로컬 좌표입니다.
    ///
    /// position이 {0, 0}이 아니라는 점이 measureText()와 다릅니다. 가운데 정렬을 예로 들면:
    ///
    /// \code{.cpp}
    /// TextParams params{.characterSize = 32};
    /// params.origin = renderer.measureTextBounds(font, text, params).center();
    /// renderer.drawText(font, text, screenCenter, params); // 회전해도 제자리
    /// \endcode
    ///
    /// 로드하지 않았거나 이동된 Font면 빈 사각형을 돌려줍니다.
    [[nodiscard]] Rectf measureTextBounds(const Font& font, std::string_view text,
                                          const TextParams& params = {}) const;

    /// 이후의 그리기를 이 카메라의 월드 좌표로 해석합니다.
    ///
    /// 카메라를 복사해 두었다가 beginFrame()마다 다시 적용하므로, 같은 카메라라면 매 프레임 다시
    /// 부를 필요가 없습니다. 카메라 값을 바꿨다면 다시 불러야 반영됩니다.
    void setCamera(const Camera2D& camera);

    /// 화면 좌표로 되돌립니다. 원점은 창의 좌상단이고 1픽셀 = 1월드유닛입니다. UI를 그리기 전에
    /// 부릅니다. 창 크기가 바뀌면 화면 좌표도 새 크기를 따르며, 처음 만든 Renderer도 이 상태입니다.
    void resetCamera();

    /// 지금 적용 중인 카메라입니다. resetCamera() 상태(화면 좌표)면 std::nullopt입니다.
    [[nodiscard]] std::optional<Camera2D> camera() const;

    /// 지금 적용 중인 카메라로 월드 좌표와 창 픽셀 좌표를 서로 바꿉니다. 정수로 자르지 않은
    /// 실수를 돌려줍니다.
    ///
    /// 마우스 위치로 월드의 물체를 고를 때는 screenToWorld()를 씁니다. 마우스 위치(Vec2i)는
    /// Vec2f로 바꿔 넘깁니다.
    /// \{
    [[nodiscard]] Vec2f worldToScreen(Vec2f worldPoint) const;
    [[nodiscard]] Vec2f screenToWorld(Vec2f screenPoint) const;
    /// \}

    /// 지금 카메라로 보이는 월드 영역입니다. 화면 밖 오브젝트를 건너뛸(컬링) 때 씁니다.
    ///
    /// 카메라가 회전해 있으면 기울어진 보이는 영역을 감싸는 축 정렬 사각형을 돌려줍니다. 실제보다
    /// 조금 넓을 수는 있어도 좁지는 않으므로, 컬링에 써도 보이는 오브젝트를 놓치지 않습니다.
    [[nodiscard]] Rectf visibleWorldBounds() const;

    /// 창 그리기 영역의 픽셀 크기입니다. Window::size()와 같은 값이며, viewportPixels() 같은
    /// Camera2D 계산 함수의 framebuffer 인자로 넘깁니다.
    [[nodiscard]] Vec2u framebufferSize() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/// \}

} // namespace meno

#endif // MENO_RENDERER_HPP
