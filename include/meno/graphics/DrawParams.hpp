#ifndef MENO_DRAW_PARAMS_HPP
#define MENO_DRAW_PARAMS_HPP

#include <meno/math/Color.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

#include <optional>

namespace meno {

/// \addtogroup graphics
/// \{

// 이 파일의 Params 구조체는 모두 지정 초기화로 쓰므로, 필드 순서를 바꾸면 사용자 코드가 깨진다.
// 순서 변경은 API 변경으로 취급할 것.
/// Renderer::draw()로 텍스처를 그릴 때의 옵션입니다.
///
/// 모든 필드에 기본값이 있으므로 지정 초기화로 필요한 것만 정합니다. 이때 필드는 선언
/// 순서(position, origin, scale, rotation, tint, source, flipX, flipY)대로 써야 합니다.
///
/// \code{.cpp}
/// renderer.draw(texture, {.position = {100.f, 50.f}, .rotation = 45.f});
/// \endcode
///
/// \see Renderer::draw
struct SpriteParams {
    /// origin이 놓일 월드 좌표입니다.
    Vec2f position{0.f, 0.f};

    /// 회전과 배율의 기준점입니다. 배율을 적용하기 전의 텍스처 픽셀 단위입니다.
    ///
    /// {0, 0}이면 좌상단(source를 쓰면 그 영역의 좌상단)을, 그리는 영역 크기의 절반이면 중심을
    /// 기준으로 회전합니다.
    Vec2f origin{0.f, 0.f};

    /// 가로·세로 배율입니다. 음수 배율 대신 flipX·flipY로 뒤집으면 origin이 그대로입니다.
    Vec2f scale{1.f, 1.f};

    /// 회전 각도(도 단위)입니다. 시계 방향이 양수이며, origin을 중심으로 회전합니다.
    float rotation{0.f};

    /// 텍스처 색에 곱해지는 색입니다. 흰색이면 원본 그대로이고, alpha를 낮추면 반투명해집니다.
    Color tint{colors::White};

    /// 텍스처의 일부만 그릴 때 그 영역(텍스처 픽셀 좌표)입니다. 스프라이트 시트나 애니메이션
    /// 프레임에 씁니다. 비어 있으면 텍스처 전체를 그립니다.
    std::optional<Recti> source{}; // NOLINT(readability-redundant-member-init)

    /// 좌우(flipX)나 상하(flipY)로 뒤집어 그립니다. 그리는 위치는 그대로입니다.
    /// \{
    bool flipX{false};
    bool flipY{false};
    /// \}
};

/// drawRect(), drawCircle(), drawPolygon(), drawSector()의 옵션입니다.
///
/// 필드는 선언 순서(fill, outline, outlineThickness, origin, rotation)대로 지정해야 합니다.
struct ShapeParams {
    /// 채우기 색입니다.
    Color fill{colors::White};
    /// 외곽선 색입니다. 기본값이 투명이므로, outlineThickness만 바꾸면 외곽선이 보이지 않습니다.
    Color outline{colors::Transparent};

    /// 외곽선 두께입니다. 양수면 바깥쪽, 음수면 안쪽으로 그리며 0이면 외곽선이 없습니다.
    float outlineThickness{0.f};

    /// position에 놓일 점이자 회전 중심입니다. 기준은 도형마다 다릅니다.
    ///
    /// drawRect()와 drawPolygon()에서는 도형의 로컬 원점(사각형은 좌상단)이, drawCircle()과
    /// drawSector()에서는 중심이 기준입니다.
    Vec2f origin{0.f, 0.f};
    /// 회전 각도(도 단위)입니다. 시계 방향이 양수이며, origin을 중심으로 회전합니다.
    float rotation{0.f};
};

/// drawLine()의 옵션입니다.
struct LineParams {
    /// 선 색입니다.
    Color color{colors::White};

    /// 선 두께입니다. 선을 가운데에 두고 양쪽으로 반씩 퍼지며, 양의 유한수가 아니면 그리지
    /// 않습니다.
    float thickness{1.f};
};

/// drawText(), measureText(), measureTextBounds()의 옵션입니다.
///
/// 필드는 선언 순서(characterSize, color, origin, rotation, outlineThickness, outlineColor)대로
/// 지정해야 합니다.
struct TextParams {
    /// 글자 크기입니다. 화면 좌표에서는 픽셀 단위입니다.
    unsigned int characterSize{24};

    /// 글자 색입니다.
    Color color{colors::White};

    /// position에 놓일 점이자 회전 중심입니다. 텍스트 상자의 좌상단에서 잽니다.
    /// 가운데 정렬하려면 Renderer::measureTextBounds()의 중심을 넣습니다.
    Vec2f origin{0.f, 0.f};
    /// 회전 각도(도 단위)입니다. 시계 방향이 양수이며, origin을 중심으로 회전합니다.
    float rotation{0.f};

    /// 외곽선 두께입니다. 0이면 외곽선이 없습니다.
    float outlineThickness{0.f};
    /// 외곽선 색입니다.
    Color outlineColor{colors::Black};
};

/// \}

} // namespace meno

#endif // MENO_DRAW_PARAMS_HPP
