#ifndef MENO_RENDERER_HPP
#define MENO_RENDERER_HPP

#include <meno/math/Vec2.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Camera2D.hpp>
#include <meno/graphics/DrawParams.hpp>

#include <memory>
#include <string_view>

namespace meno {

class Font;
class Texture;
class Window;

// 2D 렌더 API.
//
// 즉시 모드다. draw를 호출한 순서가 곧 그려지는 순서이고(나중 호출이 위),
// 내부에 커맨드 버퍼나 정렬 단계가 없다. 레이어가 필요하면 호출 순서로
// 표현한다 -- 배경 먼저, 캐릭터 나중.
//
// 한 프레임의 형태:
//
//     renderer.beginFrame(colors::Black);
//     renderer.draw(playerTexture, {.position = {100.f, 50.f}});
//     renderer.endFrame();
//
// 이 헤더는 SFML을 include하지 않는다. 사용자 빌드에 SFML 헤더 경로가 없어도 컴파일된다.
class Renderer {
public:
    // window는 Renderer보다 오래 살아야 한다. 소유하지 않는다.

    // 생성 시점에 window의 렌터 타깃을 붙잡는다.
    // 이후 window에 다른 Window를 이동 대입하면 붙잡은 타깃이 파괴되고, Renderer도 잇따라 새로 만들어야 한다.
    // 렌더 타깃이 없는 Window(이동당한 빈 껍데기)를 넘기면 std::invalid_argument를 던진다.
    explicit Renderer(Window& window);
    ~Renderer();

    // 위에 따라, Renderer의 복사와 이동 모두 금지한다.
    // Window를 빌려 쓰는 타입이라 옮길 이유가 거의 없고,
    // 이동을 열어두면 이동당한 쪽(impl_ == nullptr)을 모든 함수가 굳이 검사해야 한다.
    // 복사와 이동을 막아두면 "살아있는 Renderer는 항상 유효한 렌더 타깃을 갖는다"는 불변식이 자료형으로 보장된다.

    // 혹시 여러 곳에서 사용해야 한다면 Renderer&로 넘긴다.
    // 생성을 미뤄야 할 경우에는 std::optional<Renderer>::emplace나 std::unique_ptr<Renderer>를 쓴다.
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    // --- 프레임 -----------------------------------------------------------

    // 화면을 clearColor로 지우고 그리기를 시작한다.
    void beginFrame(Color clearColor = colors::Black);

    // 이번 프레임에 쌓인 그리기를 화면에 표시한다.
    void endFrame();

    // --- 스프라이트 -------------------------------------------------------

    // 텍스처를 position에 원본 크기로 그린다. 가장 짧은 형태.
    void draw(const Texture& texture, Vec2f position);

    // 옵션을 지정해서 그린다. SpriteParams 참고.
    void draw(const Texture& texture, const SpriteParams& params);

    // --- 기본 도형 --------------------------------------------------------

    void drawRect(const Rectf& rect, const ShapeParams& params = {});
    void drawCircle(Vec2f center, float radius, const ShapeParams& params = {});
    void drawLine(Vec2f from, Vec2f to, Color color = colors::White, float thickness = 1.f);

    // --- 텍스트 -----------------------------------------------------------

    // text는 UTF-8로 해석한다. 한글 문자열 리터럴이 그대로 들어간다.
    void drawText(const Font& font, std::string_view text, Vec2f position,
                  const TextParams& params = {});

    // 그리지 않고 차지할 크기만 잰다. 같은 params로 drawText한 결과와 크기가 같다.

    // 주의: 글자의 실제 잉크는 drawText의 position에서 바로 시작하지 않는다.
    // 윗부분 여백(글리프 베어링)과 외곽선만큼 어긋나 있어서, 크기만으로 정렬하면 몇 픽셀 틀어진다.
    // 픽셀 단위로 세심히 맞추고 싶을 경우, 아래의 measureTextBounds를 쓰자.
    [[nodiscard]] Vec2f measureText(const Font& font, std::string_view text,
                                    const TextParams& params = {}) const;

    // drawText(font, text, {0, 0}, params)로 그렸을 때, 실제 잉크가 차지하는 영역을 Rectf로 반환한다.
    // params의 origin과 rotation을 적용하기 전의 로컬 좌표이다.

    // position이 {0, 0}이 아니라는 점이 measureText와 다르다. 가운데 정렬을 예로 들면:
    //     TextParams params{.characterSize = 32};
    //     params.origin = renderer.measureText(font, text, params).center();
    //     renderer.drawText(font, text, screenCenter, params);   // 회전해도 제자리

    // 로드되지 않거나 이동된 폰트라면 빈 사각형을 돌려준다.
    [[nodiscard]] Rectf measureTextBounds(const Font& font, std::string_view text,
                                          const TextPrams& params = {}) const;

    // --- 카메라 -----------------------------------------------------------

    void setCamera(const Camera2D& camera);

    // 1픽셀 = 1월드유닛인 기본 시점으로 되돌린다. UI를 그리기 전에 호출한다.
    void resetCamera();

    [[nodiscard]] Vec2f worldToScreen(Vec2f worldPoint) const;
    [[nodiscard]] Vec2f screenToWorld(Vec2f screenPoint) const;

    // 현재 카메라로 보이는 월드 영역. 화면 밖 오브젝트를 건너뛸 때 쓴다.

    // 카메라가 회전해 있으면 보이는 영역은 기울어진 사각형이 된다.
    // 이때 그것을 감싸는 축 정렬 사각형을 돌려준다. (회전된 사각형의 외접 사각형)
    // 실제보다 조금 넓을 수는 잇어도 좁지는 않으므로,
    // 컬링에 써도 보이는 오브젝트를 놓치지 않는다.
    [[nodiscard]] Rectf visibleWorldBounds() const;

    // --- 정보 -------------------------------------------------------------

    [[nodiscard]] Vec2u framebufferSize() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace meno

#endif  // MENO_RENDERER_HPP