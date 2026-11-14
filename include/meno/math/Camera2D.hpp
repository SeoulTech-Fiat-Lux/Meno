#ifndef MENO_CAMERA2D_HPP
#define MENO_CAMERA2D_HPP

#include <meno/math/Angle.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace meno {

/// \addtogroup math
/// \{

/// 카메라가 담는 월드 영역과 뷰포트의 종횡비가 다를 때, 둘을 어떻게 맞출지 정합니다.
enum class CameraFit : std::uint8_t {
    // SFML의 기본 동작(sf::View)과 같다.
    /// 카메라가 담는 월드 영역을 뷰포트에 그대로 맞춥니다.
    ///
    /// 종횡비가 뷰포트와 다르면 그림이 늘어나거나 찌그러집니다.
    /// 띠 없이 뷰포트를 꽉 채워야 할 때 지정합니다. (.fit = CameraFit::Stretch)
    Stretch,

    /// 종횡비를 유지하면서 카메라가 담는 월드 영역을 뷰포트 안에 최대한 크게 맞춥니다.
    /// Camera2D의 기본값입니다.
    ///
    /// 남는 영역(띠)은 Renderer::beginFrame()의 clearColor로 채워집니다. 띠와 배경의 색을 다르게
    /// 하려면 배경을 사각형으로 먼저 그립니다.
    /// size가 {0, 0}이면 맞출 종횡비가 없으므로 Stretch와 똑같이 동작합니다.
    ///
    /// 띠 위의 마우스 좌표를 screenToWorld에 넣으면 보이는 영역 밖의 월드 좌표가 나옵니다.
    /// 이를 거르려면 viewportPixels(camera, framebuffer).contains(Vec2f(mouse))로 먼저 확인합니다.
    Letterbox,
};

// SFML을 담지 않는 순수 데이터라 pimpl이 필요 없다.
// 지정 초기화는 선언 순서를 따라야 하므로, 필드 순서를 바꾸면 사용자 코드가 깨진다.
/// 월드의 어느 영역을 창의 어디에 얼마나 크게 비출지 정하는 2D 카메라입니다.
///
/// 값 타입입니다. Renderer::setCamera()는 복사본을 저장하므로, 카메라를 바꾼 뒤에는
/// setCamera()를 다시 호출해야 화면에 반영됩니다. 지정 초기화는 필드 순서(center, size, zoom,
/// rotation, viewport, fit)대로 써야 합니다.
///
/// \code{.cpp}
/// meno::Camera2D camera{.center = {160.f, 90.f}, .size = {320.f, 180.f}};
/// // UI용 카메라입니다. size가 같고 움직이지 않으므로 UI가 게임 화면에 고정됩니다.
/// const meno::Camera2D hud{.center = {160.f, 90.f}, .size = {320.f, 180.f}};
/// camera.center = playerPosition; // 플레이어를 따라갑니다.
/// renderer.setCamera(camera);
/// // 월드를 그립니다.
/// renderer.setCamera(hud);
/// // UI를 그립니다.
/// \endcode
///
/// \see Renderer::setCamera, viewportPixels, worldToScreen, screenToWorld
struct Camera2D {
    /// 화면(뷰포트) 중앙에 올 월드 좌표입니다.
    Vec2f center{0.f, 0.f};

    /// 화면에 담을 월드 영역의 크기("설계 해상도")입니다.
    ///
    /// 기본값 {0, 0}처럼 양의 유한수가 아닌 축이 하나라도 있으면 뷰포트의 픽셀 크기를 그대로
    /// 씁니다. 이때 1월드유닛은 1픽셀과 같습니다.
    Vec2f size{0.f, 0.f};

    /// 화면 배율입니다. 보이는 영역의 크기는 size / zoom입니다.
    ///
    /// 1보다 크면 확대(보이는 영역이 좁아짐), 작으면 축소입니다. size는 고정하고 줌 연출은
    /// zoom으로 하면 둘이 섞이지 않습니다. 양의 유한수가 아니면(0 이하, NaN, 무한대) 1로 봅니다.
    float zoom{1.f};

    /// 카메라의 회전 각도(도). 시계 방향이 양수이며 범위 제한은 없습니다.
    ///
    /// 카메라가 시계 방향으로 돌면 화면에 비친 월드는 반시계 방향으로 돕니다. 같은 +45라도
    /// 스프라이트(SpriteParams::rotation)는 시계 방향으로 회전하므로 혼동하지 않도록 주의합니다.
    ///
    /// 화면 위쪽이 가리키는 월드 방향:
    ///
    ///     rotation ==   0 -> 월드 -y (회전 없음. y가 아래로 커지므로 위쪽은 -y)
    ///     rotation ==  90 -> 월드 +x
    ///     rotation == 180 -> 월드 +y
    ///     rotation == -90 -> 월드 -x (== 270)
    float rotation{0.f};

    /// 창에서 이 카메라가 그릴 영역. 창 전체를 1로 보는 비율 좌표입니다.
    ///
    /// 기본값 {0, 0, 1, 1}은 창 전체입니다. 화면 분할, 미니맵 등에 씁니다.
    /// 실제 픽셀 영역은 viewportPixels()로 구합니다.
    ///
    /// Letterbox는 뷰포트마다 따로 적용되므로, 창을 반씩 나눈 두 화면은 각자 띠를 가집니다.
    /// Letterbox를 적용한 게임 화면 안을 나누려면 viewportPixels()로 게임 화면의 픽셀 영역을
    /// 구하고, 그 안에서 비율을 계산합니다.
    ///
    /// \code{.cpp}
    /// .viewport = {0.f, 0.f, 0.5f, 1.f}      // 왼쪽 절반
    /// .viewport = {0.75f, 0.f, 0.25f, 0.25f} // 우상단 미니맵
    /// \endcode
    Rectf viewport{{0.f, 0.f}, {1.f, 1.f}};

    // size를 정했다는 것은 "이 해상도로 설계했다"는 뜻이므로, 따로 지시하지 않아도
    // 비율을 지키도록 Letterbox를 기본값으로 둔다.
    /// size와 viewport의 종횡비가 다를 때의 처리입니다. 기본은 Letterbox입니다.
    ///
    /// \see CameraFit
    CameraFit fit{CameraFit::Letterbox};
};

namespace detail {

[[nodiscard]] inline bool isPositiveFinite(float value) {
    return std::isfinite(value) && value > 0.f;
}

} // namespace detail

/// 카메라가 실제로 그리는 창 안의 픽셀 영역. 창 좌상단이 {0, 0}입니다.
///
/// viewport 비율에 창 크기를 곱하고, CameraFit::Letterbox면 종횡비에 맞게 줄여 가운데에 놓습니다.
/// 결과는 정수 픽셀 경계에 맞춰지며, 맞닿은 두 뷰포트 사이에는 틈도 겹침도 없습니다.
/// viewport에 NaN이나 무한대가 있거나 크기가 음수면 크기가 0인 사각형을 돌려주므로 아무것도
/// 그려지지 않습니다.
///
/// \param framebuffer 창(렌더 타깃)의 픽셀 크기. 보통 Renderer::framebufferSize()를 넘깁니다.
[[nodiscard]] inline Rectf viewportPixels(const Camera2D& camera, Vec2u framebuffer) {
    const Rectf& ratio = camera.viewport;
    const double screenWidth = framebuffer.x;
    const double screenHeight = framebuffer.y;

    double left = static_cast<double>(ratio.left()) * screenWidth;
    double top = static_cast<double>(ratio.top()) * screenHeight;
    double right = static_cast<double>(ratio.left() + ratio.size.x) * screenWidth;
    double bottom = static_cast<double>(ratio.top() + ratio.size.y) * screenHeight;
    if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) ||
        !std::isfinite(bottom)) {
        return {};
    }
    right = std::max(right, left);
    bottom = std::max(bottom, top);

    if (camera.fit == CameraFit::Letterbox && detail::isPositiveFinite(camera.size.x) &&
        detail::isPositiveFinite(camera.size.y)) {
        // 두 축 중 덜 늘어나는 쪽에 맞춰야 size 전체가 뷰포트 안에 들어간다.
        const double width = right - left;
        const double height = bottom - top;
        const double scale = std::min(width / camera.size.x, height / camera.size.y);
        const double fittedWidth = camera.size.x * scale;
        const double fittedHeight = camera.size.y * scale;
        left += (width - fittedWidth) / 2.0;
        top += (height - fittedHeight) / 2.0;
        right = left + fittedWidth;
        bottom = top + fittedHeight;
    }

    // 네 모서리를 각각 반올림한다. SFML처럼 위치와 크기를 따로 반올림하면, 창을 3등분한 화면
    // 사이에 1픽셀 틈이 생긴다. 모서리를 반올림하면 맞닿은 두 뷰포트가 같은 경계를 공유하므로
    // 틈도 겹침도 없다.
    const double roundedLeft = std::round(left);
    const double roundedTop = std::round(top);
    const double roundedRight = std::max(std::round(right), roundedLeft);
    const double roundedBottom = std::max(std::round(bottom), roundedTop);
    return Rectf{static_cast<float>(roundedLeft), static_cast<float>(roundedTop),
                 static_cast<float>(roundedRight - roundedLeft),
                 static_cast<float>(roundedBottom - roundedTop)};
}

/// 회전을 적용하기 전, 카메라가 담는 월드 사각형입니다.
///
/// center를 중심으로 하고 크기는 size / zoom입니다. size가 {0, 0}이면 viewportPixels()의 크기를
/// 씁니다. 결과의 크기는 항상 양수입니다.
///
/// \param framebuffer 창(렌더 타깃)의 픽셀 크기. 보통 Renderer::framebufferSize()를 넘깁니다.
[[nodiscard]] inline Rectf viewRect(const Camera2D& camera, Vec2u framebuffer) {
    Vec2f size = camera.size;
    if (!detail::isPositiveFinite(size.x) || !detail::isPositiveFinite(size.y)) {
        size = viewportPixels(camera, framebuffer).size;
    }

    const float zoom = detail::isPositiveFinite(camera.zoom) ? camera.zoom : 1.f;
    size /= zoom;
    // 뷰포트가 비어 크기가 0이 될 때는 1로 두어 0으로 나누지 않게 한다.
    // 뷰포트가 비어 있으면 어차피 아무것도 그려지지 않는다.
    if (!detail::isPositiveFinite(size.x)) {
        size.x = 1.f;
    }
    if (!detail::isPositiveFinite(size.y)) {
        size.y = 1.f;
    }
    return Rectf{camera.center - size / 2.f, size};
}

/// 카메라에 보이는 월드 영역. 화면 밖 오브젝트를 건너뛸(컬링) 때 씁니다.
///
/// 카메라가 회전해 있으면 기울어진 보이는 영역을 감싸는 축 정렬 사각형을 돌려줍니다. 실제보다
/// 조금 넓을 수는 있어도 좁지는 않으므로, 컬링에 써도 보이는 오브젝트를 놓치지 않습니다.
///
/// \param framebuffer 창(렌더 타깃)의 픽셀 크기. 보통 Renderer::framebufferSize()를 넘깁니다.
[[nodiscard]] inline Rectf visibleWorldBounds(const Camera2D& camera, Vec2u framebuffer) {
    // 폭 w, 높이 h를 θ만큼 돌리면 외접 사각형은 (w|cosθ| + h|sinθ|, w|sinθ| + h|cosθ|)이다.
    // 예: 45도면 두 축 모두 (w + h) / √2이고, 정사각형일 때만 정확히 √2배가 된다.
    const Vec2f size = viewRect(camera, framebuffer).size;
    const float radians = toRadians(wrapDegrees(camera.rotation));
    const float cosAbs = std::abs(std::cos(radians));
    const float sinAbs = std::abs(std::sin(radians));
    const Vec2f extent{size.x * cosAbs + size.y * sinAbs, size.x * sinAbs + size.y * cosAbs};
    return Rectf{camera.center - extent / 2.f, extent};
}

/// 월드 좌표 -> 창 픽셀 좌표(창 좌상단 기준).
///
/// 정수로 자르지 않은 실수를 돌려줍니다. 뷰포트가 창의 일부라면 결과도 그 뷰포트 안으로
/// 들어갑니다.
///
/// \param framebuffer 창(렌더 타깃)의 픽셀 크기. 보통 Renderer::framebufferSize()를 넘깁니다.
[[nodiscard]] inline Vec2f worldToScreen(const Camera2D& camera, Vec2u framebuffer,
                                         Vec2f worldPoint) {
    const Rectf pixels = viewportPixels(camera, framebuffer);
    const Vec2f size = viewRect(camera, framebuffer).size;
    const float radians = toRadians(wrapDegrees(camera.rotation));
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    // 카메라 회전의 역회전으로 월드 좌표를 카메라 기준 좌표로 바꾼 뒤 뷰포트 픽셀로 늘린다.
    const Vec2f offset = worldPoint - camera.center;
    const Vec2f local{cosine * offset.x + sine * offset.y, -sine * offset.x + cosine * offset.y};
    return Vec2f{pixels.left() + (local.x / size.x + 0.5f) * pixels.size.x,
                 pixels.top() + (local.y / size.y + 0.5f) * pixels.size.y};
}

/// 창 픽셀 좌표 -> 월드 좌표.
///
/// worldToScreen의 역함수입니다. 마우스 위치로 월드의 물체를 고를 때 씁니다.
/// 뷰포트 밖의 픽셀을 넣어도 같은 규칙을 연장해서 계산합니다.
/// 뷰포트가 비어 있으면 역변환이 없으므로 center를 돌려줍니다.
///
/// \param framebuffer 창(렌더 타깃)의 픽셀 크기. 보통 Renderer::framebufferSize()를 넘깁니다.
[[nodiscard]] inline Vec2f screenToWorld(const Camera2D& camera, Vec2u framebuffer,
                                         Vec2f screenPoint) {
    const Rectf pixels = viewportPixels(camera, framebuffer);
    if (pixels.size.x <= 0.f || pixels.size.y <= 0.f) {
        return camera.center;
    }

    const Vec2f size = viewRect(camera, framebuffer).size;
    const float radians = toRadians(wrapDegrees(camera.rotation));
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    const Vec2f local{((screenPoint.x - pixels.left()) / pixels.size.x - 0.5f) * size.x,
                      ((screenPoint.y - pixels.top()) / pixels.size.y - 0.5f) * size.y};
    return camera.center +
           Vec2f{cosine * local.x - sine * local.y, sine * local.x + cosine * local.y};
}

/// \}

} // namespace meno

#endif // MENO_CAMERA2D_HPP
