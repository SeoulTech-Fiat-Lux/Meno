#ifndef MENO_CAMERA2D_HPP
#define MENO_CAMERA2D_HPP

#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace meno {
// 카메라가 담는 월드 영역과 뷰포트의 종횡비가 다를 때, 둘을 어떻게 맞출지 정한다.
enum class CameraFit : std::uint8_t {
    // SFML의 기본 동작과 동일하게, 카메라가 담는 월드 영역을 뷰포트에 맞춘다.
    // 이때 카메라가 담는 월드 영역의 종횡비가 뷰포트와 다르면, 그림이 늘어나거나 찌그러진다.
    // 띠 없이 뷰포트를 꽉 채워야 할 때 명시한다. (.fit = CameraFit::Stretch)
    Stretch,

    // 종횡비를 유지하면서 뷰포트 안에 카메라가 담는 월드 영역을 최대한 맞춘다. Camera2D의 기본값이다.
    // 남는 영역은 beginFrame의 clearColor로 채워진다.
    // size가 {0, 0}이면, 맞춰야 할 종횡비가 없으므로 Stretch와 동일하게 작동한다.
    // 띠 위의 마우스 좌표를 screenToWorld에 넣으면 보이는 영역 밖의 월드 좌표가 나온다.
    // 걸러야 하면 viewportPixels(camera, framebuffer).contains(mouse)로 먼저 확인한다.
    Letterbox,
};

// 화면 상에 월드의 어느 영역을 얼만큼 비출지 기술하는 2D 카메라.
// SFML을 담지 않는 순수 데이터라 pimpl이 필요 없다.
// 값으로 자유롭게 복사하고 게임 쪽에서 들고 있다가, Renderer::setCamera로 넘기면 된다.
// 지정 초기화는 선언 순서를 지켜야 하므로, 필드 순서를 바꾸면 사용자 코드가 깨진다.
struct Camera2D {
    // 화면(뷰포트) 중앙에 올 월드의 좌표.
    Vec2f center{0.f, 0.f};

    // 화면에 담을 월드 영역의 크기. "설계 해상도"를 정하는 값이다.
    // Camera2D::size == Vec2f{0.f, 0.f} -> 월드 유닛 == 1픽셀
    // 크기를 정하지 않으면 뷰포트의 픽셀 크기를 그대로 가져다 쓰기 때문에, 월드 유닛의 크기가 1픽셀의 크기와 같아진다.
    Vec2f size{0.f, 0.f};

    // 화면의 배율(magnification). size 위에 곱해지는 "동적 확대"다.
    // 1보다 크면 확대(보이는 영역이 좁아짐), 작으면 축소. 보이는 영역 = size / zoom.
    // size는 게임 내내 고정하고, 연출(줌 인/아웃)은 zoom으로 하면 둘이 섞이지 않는다.
    // 0 이하이거나 NaN이면 1로 본다.
    float zoom{1.f};

    // 카메라의 회전 각도(degree). 시계 방향이 양수이며, 범위 제한은 없다. (360을 넘거나 음수여도 된다.)
    // 카메라가 시계 방향으로 돌면, 화면에 비친 월드는 반대로 "반시계 방향"으로 돈다.
    // 같은 +45를 줘도 스프라이트(SpriteParams::rotation)는 시계 방향으로 돌아가니 혼동하지 말 것.

    // 화면 위쪽이 가리키는 월드 방향:
    //     rotation ==   0 -> 월드 -y (회전 없음. y축은 아래로 증가하므로 위쪽은 -y)
    //     rotation ==  90 -> 월드 +x
    //     rotation == 180 -> 월드 +y
    //     rotation == -90 -> 월드 -x (== 270)
    float rotation{0.f};

    // 창에서 이 카메라가 그릴 영역. 창 전체를 1로 보는 비율 좌표다.
    // 기본값 {0, 0, 1, 1}은 창 전체. 화면 분할, 미니맵 등에 쓴다.
    //     .viewport = {0.f, 0.f, 0.5f, 1.f}   // 왼쪽 절반
    //     .viewport = {0.75f, 0.f, 0.25f, 0.25f} // 우상단 미니맵

    // 실제 픽셀은 네 모서리를 각각 가장 가까운 정수로 반올림해서 정한다.
    Rectf viewport{{0.f, 0.f}, {1.f, 1.f}};

    // size와 viewport의 종횡비가 다를 때의 처리.
    // 기본은 Letterbox. size를 정했다는 것은 "이 해상도로 설계했다"는 뜻이므로, 따로 지시하지 않아도 비율을 지킨다.
    // 찌그러지더라도 뷰포트를 꽉 채우려면 Stretch를 명시한다. (SFML의 sf::View와 같은 동작)
    CameraFit fit{CameraFit::Letterbox};
};

// --- 카메라 수학 ------------------------------------------------------------
// framebuffer는 창(렌더 타깃)의 픽셀 크기다. 보통 Renderer::framebufferSize()를 넘긴다.
// Renderer의 같은 이름 멤버 함수들은 이 함수들에 현재 카메라와 창 크기를 넣어 부르는 얇은 껍데기다.

namespace detail {

inline constexpr float kPi = 3.14159265358979323846f;

// 유한한 양수인지 확인하는 인라인 함수. NaN, 무한대, 0 이하를 한 번에 거른다.
[[nodiscard]] inline bool isPositiveFinite(float value) {
    return std::isfinite(value) && value > 0.f;
}

// 각도를 [0, 360)으로 접는다.
// 큰 각도를 그대로 라디안으로 바꾸면 float 정밀도가 모자라 sin/cos가 어긋난다. (1e7도에서 3픽셀 차이)
// fmod는 오차 없이 계산되므로 먼저 접어 두면 몇 바퀴를 돌았든 같은 결과가 나온다.
// backend::toSfView도 이 값을 SFML에 넘겨서, 그리는 쪽과 계산하는 쪽의 각도가 일치하도록 한다.
[[nodiscard]] inline float wrapDegrees(float degrees) {
    if (!std::isfinite(degrees)) {
        return 0.f;
    }
    float wrapped = std::fmod(degrees, 360.f);
    if (wrapped < 0.f) {
        wrapped += 360.f;
    }
    // -1e-6 + 360처럼 더한 결과가 반올림되어 정확히 360이 되는 경우.
    return wrapped < 360.f ? wrapped : 0.f;
}

[[nodiscard]] inline float toRadians(float degrees) {
    return degrees * (kPi / 180.f);
}

} // namespace detail

// 카메라가 실제로 그리는 창 안의 픽셀 영역. 창 좌상단이 {0, 0}이다.
// viewport 비율에 창 크기를 곱한 뒤, CameraFit::Letterbox면 종횡비에 맞게 줄여 가운데에 놓는다.

// 마지막으로 네 모서리를 각각 정수로 반올림하므로 결과는 항상 정수 값이다.
// SFML처럼 위치와 크기를 따로 반올림하면, 창을 3등분한 화면 사이에 1픽셀 틈이 생긴다.
// 모서리를 반올림하면 맞닿은 두 뷰포트가 같은 경계를 공유하므로 틈도 겹침도 없다.

// viewport에 NaN이나 무한대가 있거나 크기가 음수면 빈 사각형이 되어 아무것도 그려지지 않는다.
[[nodiscard]] inline Rectf viewportPixels(const Camera2D& camera, Vec2u framebuffer) {
    const Rectf& ratio = camera.viewport;
    const auto screen = static_cast<Vec2f>(framebuffer);

    float left = ratio.left() * screen.x;
    float top = ratio.top() * screen.y;
    float width = std::max(ratio.size.x * screen.x, 0.f);
    float height = std::max(ratio.size.y * screen.y, 0.f);
    if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(width) || !std::isfinite(height)) {
        return {};
    }

    if (camera.fit == CameraFit::Letterbox && detail::isPositiveFinite(camera.size.x) &&
        detail::isPositiveFinite(camera.size.y)) {
        // 두 축 중 덜 늘어나는 쪽에 맞춰야 size 전체가 뷰포트 안에 들어간다.
        const float scale = std::min(width / camera.size.x, height / camera.size.y);
        const float fittedWidth = camera.size.x * scale;
        const float fittedHeight = camera.size.y * scale;
        left += (width - fittedWidth) / 2.f;
        top += (height - fittedHeight) / 2.f;
        width = fittedWidth;
        height = fittedHeight;
    }

    const float roundedLeft = std::round(left);
    const float roundedTop = std::round(top);
    const float roundedRight = std::max(std::round(left + width), roundedLeft);
    const float roundedBottom = std::max(std::round(top + height), roundedTop);
    return Rectf{roundedLeft, roundedTop, roundedRight - roundedLeft, roundedBottom - roundedTop};
}

// 회전을 적용하기 전, 카메라가 담는 월드 사각형. center를 중심으로 하고 크기는 size / zoom이다.
// size가 {0, 0}이면 viewportPixels의 크기를 쓴다. (1픽셀 = 1월드유닛)

// 결과의 크기는 항상 양수다. 뷰포트가 비어 크기가 0이 될 때는 1로 두어 0으로 나누지 않게 한다.
// 뷰포트가 비어 있으면 어차피 아무것도 그려지지 않는다.
[[nodiscard]] inline Rectf viewRect(const Camera2D& camera, Vec2u framebuffer) {
    Vec2f size = camera.size;
    if (!detail::isPositiveFinite(size.x) || !detail::isPositiveFinite(size.y)) {
        size = viewportPixels(camera, framebuffer).size;
    }

    const float zoom = detail::isPositiveFinite(camera.zoom) ? camera.zoom : 1.f;
    size /= zoom;
    if (!detail::isPositiveFinite(size.x)) {
        size.x = 1.f;
    }
    if (!detail::isPositiveFinite(size.y)) {
        size.y = 1.f;
    }
    return Rectf{camera.center - size / 2.f, size};
}

// 카메라에 보이는 월드 영역. 화면 밖 오브젝트를 건너뛸(컬링) 때 쓴다.

// 카메라가 회전해 있으면 보이는 영역은 기울어진 사각형이 된다.
// 이때 그것을 감싸는 축 정렬 사각형을 돌려준다. (회전된 사각형의 외접 사각형)
// 실제보다 조금 넓을 수는 있어도 좁지는 않으므로, 컬링에 써도 보이는 오브젝트를 놓치지 않는다.

// 폭 w, 높이 h를 θ만큼 돌리면 외접 사각형은 (w|cosθ| + h|sinθ|, w|sinθ| + h|cosθ|)이다.
// e.g. 45도면 두 축 모두 (w + h) / √2. 정사각형일 때만 정확히 √2배가 된다.
[[nodiscard]] inline Rectf visibleWorldBounds(const Camera2D& camera, Vec2u framebuffer) {
    const Vec2f size = viewRect(camera, framebuffer).size;
    const float radians = detail::toRadians(detail::wrapDegrees(camera.rotation));
    const float cosAbs = std::abs(std::cos(radians));
    const float sinAbs = std::abs(std::sin(radians));
    const Vec2f extent{size.x * cosAbs + size.y * sinAbs, size.x * sinAbs + size.y * cosAbs};
    return Rectf{camera.center - extent / 2.f, extent};
}

// 월드 좌표 -> 창 픽셀 좌표(창 좌상단 기준). 정수로 자르지 않은 실수를 돌려준다.
// 뷰포트가 창의 일부라면 결과도 그 뷰포트 안으로 들어간다.
[[nodiscard]] inline Vec2f worldToScreen(const Camera2D& camera, Vec2u framebuffer, Vec2f worldPoint) {
    const Rectf pixels = viewportPixels(camera, framebuffer);
    const Vec2f size = viewRect(camera, framebuffer).size;
    const float radians = detail::toRadians(detail::wrapDegrees(camera.rotation));
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    // 카메라 회전의 역회전으로 월드 좌표를 카메라 기준 좌표로 바꾼 뒤 뷰포트 픽셀로 늘린다.
    const Vec2f offset = worldPoint - camera.center;
    const Vec2f local{cosine * offset.x + sine * offset.y, -sine * offset.x + cosine * offset.y};
    return Vec2f{pixels.left() + (local.x / size.x + 0.5f) * pixels.size.x,
                 pixels.top() + (local.y / size.y + 0.5f) * pixels.size.y};
}

// 창 픽셀 좌표 -> 월드 좌표. worldToScreen의 역함수다. 마우스 위치로 월드의 물체를 고를 때 쓴다.
// 뷰포트 밖의 픽셀을 넣어도 같은 규칙을 연장해서 계산한다.

// 뷰포트가 비어 있으면 역변환이 없으므로 center를 돌려준다.
[[nodiscard]] inline Vec2f screenToWorld(const Camera2D& camera, Vec2u framebuffer, Vec2f screenPoint) {
    const Rectf pixels = viewportPixels(camera, framebuffer);
    if (pixels.size.x <= 0.f || pixels.size.y <= 0.f) {
        return camera.center;
    }

    const Vec2f size = viewRect(camera, framebuffer).size;
    const float radians = detail::toRadians(detail::wrapDegrees(camera.rotation));
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);

    const Vec2f local{((screenPoint.x - pixels.left()) / pixels.size.x - 0.5f) * size.x,
                      ((screenPoint.y - pixels.top()) / pixels.size.y - 0.5f) * size.y};
    return camera.center + Vec2f{cosine * local.x - sine * local.y, sine * local.x + cosine * local.y};
}

} // namespace meno

#endif  // MENO_CAMERA2D_HPP