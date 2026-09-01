#ifndef MENO_DRAWPARAMS_HPP
#define MENO_DRAWPARAMS_HPP

#include <meno/math/Color.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <optional>

namespace meno {

/// 스프라이트 그리기 옵션.
///
/// 전부 기본값이 있어서 필요한 것만 지정하면 된다. C++20 지정 초기화와
/// 함께 쓰는 것을 전제로 설계했다:
///
///     renderer.draw(tex, {.position = {100, 50}, .rotation = 45.f});
///
/// 지정 초기화는 선언 순서를 지켜야 하므로, 필드 순서를 바꾸면 사용자
/// 코드가 깨진다. 순서 변경은 API 변경으로 취급할 것.
struct SpriteParams {
    /// 월드 좌표. origin이 놓일 위치다.
    Vec2f position{0.f, 0.f};

    /// 회전/스케일의 기준점. 텍스처 픽셀 단위이며 스케일 적용 전 기준.
    /// {0,0}이면 좌상단, 텍스처 크기의 절반이면 중심 회전이 된다.
    Vec2f origin{0.f, 0.f};

    Vec2f scale{1.f, 1.f};

    /// 도(degree), 시계 방향.
    float rotation{0.f};

    /// 텍스처 색에 곱해지는 색. 흰색이면 원본 그대로다.
    /// alpha를 낮추면 반투명해진다.
    Color tint{colors::White};

    /// 텍스처의 일부만 그릴 때 지정한다(스프라이트 시트, 애니메이션 프레임).
    /// 비어 있으면 텍스처 전체.
    std::optional<Recti> source{};

    bool flipX{false};
    bool flipY{false};
};

/// 도형(사각형/원) 그리기 옵션.
struct ShapeParams {
    Color fill{colors::White};
    Color outline{colors::Transparent};

    /// 양수면 바깥쪽, 음수면 안쪽으로 그려진다.
    float outlineThickness{0.f};

    Vec2f origin{0.f, 0.f};
    float rotation{0.f};
};

/// 텍스트 그리기 옵션.
struct TextParams {
    /// 픽셀 단위 글자 크기.
    unsigned int characterSize{24};

    Color color{colors::White};

    Vec2f origin{0.f, 0.f};
    float rotation{0.f};

    float outlineThickness{0.f};
    Color outlineColor{colors::Black};
};

} // namespace meno

#endif  // MENO_DRAWPARAMS_HPP