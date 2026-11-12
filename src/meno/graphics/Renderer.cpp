#include <meno/graphics/Renderer.hpp>

#include <meno/core/Window.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Texture.hpp>

#include "sfml_backend/Access.hpp"
#include "sfml_backend/Convert.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>

namespace meno {
namespace {

// UTF-8 std::string_view를 SFML 문자열로. 한글이 깨지지 않게 하는 지점이다.
// sf::String에 std::string을 그냥 넘기면 Latin-1로 해석해서 한글이 깨진다.
[[nodiscard]] sf::String toSfString(std::string_view text) {
    return sf::String::fromUtf8(text.begin(), text.end());
}

// drawText와  measureText가 쓰는 sf::Text를 조립하는 유일한 지점.
// 둘이 따로 조립하면 한쪽에만 옵션이 빠져 잰 크기와 그린 크기가 어긋난다.
// 실제로 measureText에 외곽선이 빠져 있어서 외곽선 t짜리 텍스트가 축마다 2 * ceil(t)만큼 작게 측정되었다.
[[nodiscard]] sf::Text makeText(const sf::Font& font, std::string_view text,
                                const TextParams& params) {
    // TextParams에 필드를 추가하면 여기에만 반영할 수 있도록 한다.
    // position은 그릴 때만 필요하므로 drawText가 따로 넣는다.
    sf::Text drawable{font, toSfString(text), params.characterSize};
    drawable.setFillColor(backend::toSf(params.color));
    drawable.setOutlineColor(backend::toSf(params.outlineColor));
    drawable.setOutlineThickness(params.outlineThickness);
    drawable.setOrigin(backend::toSf(params.origin));
    drawable.setRotation(backend::toSfAngle(params.rotation));
    return drawable;
}


// 기본 도형이 공유하는 변환 설정.
template <typename Shape>
void applyShapeParams(Shape& shape, const ShapeParams& params) {
    shape.setFillColor(backend::toSf(params.fill));
    shape.setOutlineColor(backend::toSf(params.outline));
    shape.setOutlineThickness(params.outlineThickness);
    shape.setRotation(backend::toSfAngle(params.rotation));
}

// 원 한 바퀴를 몇 개의 변으로 나눌지. drawCircle과 drawSector가 함께 쓴다.
// 둘이 같아야 쿨다운처럼 359도에서 360도(drawCircle)로 넘어갈 때 매끄러움이 달라지지 않는다.
constexpr std::size_t kSegmentsPerTurn = 30;

// 점 배열을 복사하지 않고 그대로 읽는 sf::Shape.
// sf::ConvexShape는 setPoint를 부를 때마다 도형 전체를 다시 계산해서, 점 n개를 넣으면 n번 계산한다.
// 이 클래스는 생성자에서 update()를 한 번만 부른다. 이 도형을 그릴 때까지는 points가 살아 있어야 한다.
class PolygonShape final : public sf::Shape {
public:
    explicit PolygonShape(std::span<const Vec2f> points) : points_(points) { update(); }

    [[nodiscard]] std::size_t getPointCount() const override { return points_.size(); }
    [[nodiscard]] sf::Vector2f getPoint(std::size_t index) const override {
        return backend::toSf(points_[index]);
    }

private:
    std::span<const Vec2f> points_;
};

} // namespace

struct Renderer::Impl {
    // 소유하지 않는다. Window가 Renderer보다 오래 산다는 것이 계약이다.

    // Renderer가 살아있는 동안, impl_과 target은 절대 nullptr이 아니다.
    // 생성자가 nullptr을 거부하며, 동시에 이동을 막아 Renderer의 빈 껍데기는 생기지 않기 때문이다.
    // 그래서 아래 맴버 함수들은 검사 없이 역참조한다. (impl_이 nullptr이면 target도 nullptr이므로, 둘 중 하나만 검사해도 된다.)
    sf::RenderWindow* target{nullptr};

    // 마지막으로 setCamera한 카메라. 비어 있으면 resetCamera() 상태(화면 좌표)다.
    // sf::View로 바꿔 던지지 않고 원본을 기억해야, 창 크기가 바뀐 뒤에도 같은 규칙으로 다시 계산할 수 있다.
    std::optional<Camera2D> camera;

    // 지금 그리기와 좌표 계산에 쓸 카메라.
    // 화면 좌표는 "center가 창 크기의 절반이고 size가 {0,0}인 카메라"로 표현한다. 픽셀 좌표 == 월드 좌표가 된다.
    // SFML의 getDefaultView()는 창을 만들 때 한 번만 정해져 리사이즈를 따라가지 않으므로 쓰지 않는다.
    [[nodiscard]] Camera2D activeCamera() const {
        if (camera) {
            return *camera;
        }
        return Camera2D{.center = static_cast<Vec2f>(framebuffer()) / 2.f};
    }

    // 창 크기는 Window::pollEvents가 SFML 이벤트를 꺼낼 때 SFML이 갱신한다.
    [[nodiscard]] Vec2u framebuffer() const { return backend::fromSf(target->getSize()); }

    // 현재 카메라를 현재 창 크기로 SFML에 적용한다.
    // 크기가 그대로인지 비교해 건너뛰지 않는다. Core가 같은 크기로 창을 다시 만들면(sf::RenderWindow::create)
    // SFML의 뷰가 기본값으로 돌아가는데, 크기 비교로는 이를 알아챌 수 없다. setView는 값만 바꾸므로 싸다.
    void applyView() { target->setView(backend::toSfView(activeCamera(), framebuffer())); }
};

Renderer::Renderer(Window& window) : impl_(std::make_unique<Impl>()) {
    impl_->target = backend::WindowAccess::native(window);
    if (impl_->target == nullptr) {
        // 이동당한 Window를 넘겼거나, Window가 이미 소멸되어 렌더 타깃이 없는 경우.
        // 여기에서 막지 않으면 첫 draw에서 nullptr 역참조가 일어나고, 그때는 어디서 잘못된 건지 알기 어렵다.
        throw std::invalid_argument{"Renderer: window has no render target"};
    }
}

Renderer::~Renderer() = default;

// --- 프레임 ---------------------------------------------------------------

void Renderer::beginFrame(Color clearColor) {
    // 직전 프레임 이후 창 크기가 바뀌었을 수 있다. 리사이즈를 따로 통지받지 않고 매 프레임 다시 적용한다.
    impl_->applyView();
    impl_->target->clear(backend::toSf(clearColor));
}

void Renderer::endFrame() {
    impl_->target->display();
}

// --- 스프라이트 -----------------------------------------------------------

void Renderer::draw(const Texture& texture, Vec2f position) {
    draw(texture, SpriteParams{.position = position});
}

void Renderer::draw(const Texture& texture, const SpriteParams& params) {
    const sf::Texture* native = backend::TextureAccess::native(texture);
    if (native == nullptr) {
        // 로드에 실패했거나 이동당한 텍스처. 조용히 건너뛴다.
        return;
    }

    // source가 없으면 텍스처 전체.
    const Vec2u textureSize = texture.size();
    Recti source = params.source.value_or(Recti{{0, 0},
                                               {static_cast<int>(textureSize.x),
                                                static_cast<int>(textureSize.y)}});

    // SFML은 텍스처 사각형의 폭/높이가 음수면 뒤집어서 그린다.
    // 스케일을 음수로 주는 방식과 달리 origin 계산이 틀어지지 않는다.
    if (params.flipX) {
        source.position.x += source.size.x;
        source.size.x = -source.size.x;
    }
    if (params.flipY) {
        source.position.y += source.size.y;
        source.size.y = -source.size.y;
    }

    sf::Sprite sprite{*native, backend::toSf(source)};
    sprite.setOrigin(backend::toSf(params.origin));
    sprite.setPosition(backend::toSf(params.position));
    sprite.setScale(backend::toSf(params.scale));
    sprite.setRotation(backend::toSfAngle(params.rotation));
    sprite.setColor(backend::toSf(params.tint));

    impl_->target->draw(sprite);
}

// --- 기본 도형 ------------------------------------------------------------

void Renderer::drawRect(const Rectf& rect, const ShapeParams& params) {
    sf::RectangleShape shape{backend::toSf(rect.size)};
    applyShapeParams(shape, params);
    shape.setOrigin(backend::toSf(params.origin));
    shape.setPosition(backend::toSf(rect.position));

    impl_->target->draw(shape);
}

void Renderer::drawCircle(Vec2f center, float radius, const ShapeParams& params) {
    // 넘겨받은 양의 실수가 아니라면(0, 음수, NaN, 무한대 제외)면 그리지 않는다.
    if (!detail::isPositiveFinite(radius)) {
        return;
    }

    sf::CircleShape shape{radius, kSegmentsPerTurn};
    applyShapeParams(shape, params);

    // CircleShape의 기준점은 바운딩 박스의 좌상단이다. origin을 반지름만큼
    // 옮겨야 인자로 받은 center가 실제 중심이 된다. params.origin은 그 위에
    // 얹히는 추가 오프셋으로 동작한다.
    shape.setOrigin(backend::toSf(Vec2f{radius, radius} + params.origin));
    shape.setPosition(backend::toSf(center));

    impl_->target->draw(shape);
}

void Renderer::drawLine(Vec2f from, Vec2f to, Color color, float thickness) {
    const Vec2f delta = to - from;
    const float length = delta.length();
    if (length <= 0.f) {
        return;
    }

    // 두께 있는 선은 회전시킨 사각형으로 그린다. VertexArray의 Lines는
    // 두께를 지원하지 않아서 thickness 인자가 무시되기 때문이다.
    sf::RectangleShape shape{sf::Vector2f{length, thickness}};
    shape.setFillColor(backend::toSf(color));
    shape.setOrigin(sf::Vector2f{0.f, thickness / 2.f});
    shape.setPosition(backend::toSf(from));
    shape.setRotation(sf::radians(std::atan2(delta.y, delta.x)));

    impl_->target->draw(shape);
}

void Renderer::drawPolygon(Vec2f position, std::span<const Vec2f> points, const ShapeParams& params) {
    if (points.size() < 3) {
        return;
    }

    PolygonShape shape{points};
    applyShapeParams(shape, params);
    shape.setOrigin(backend::toSf(params.origin));
    shape.setPosition(backend::toSf(position));

    impl_->target->draw(shape);
}

void Renderer::drawPolygon(Vec2f position, std::initializer_list<Vec2f> points, const ShapeParams & params) {
    drawPolygon(position, std::span<const Vec2f>{points.begin(), points.size()}, params);
}

void Renderer::drawSector(Vec2f center, float radius, float startDeg, float sweepDeg, const ShapeParams& params) {
    // NaN과 비교할 때는 항상 거짓을 반환하므로, sweepDeg가 NaN이 되는 경우 역시 이 조건문에서 걸러진다.
    if (!detail::isPositiveFinite(radius) || !(std::fabs(sweepDeg) > 0.f)) {
        return;
    }

    // 한 바퀴 이상 회전하며 그리는 경우.
    // 아래처럼 중심을 꼭짓점으로 넣으면 외곽선에 반지름 선이 하나 남게 된다.
    if (std::fabs(sweepDeg) >=360.f) {
        drawCircle(center, radius, params);
        return;
    }

    // 가장 처음 조건에서 |sweepDeg| < 360을 보장하므로 항상 kSegmentsPerTurn을 넘지 않는다.
    // 이는 아래 배열 크기의 근거가 되므로, clamp로 수치를 못박아 둔다.
    const float turns = std::fabs(sweepDeg) / 360.f;
    const auto segments = std::clamp<std::size_t>(
        static_cast<std::size_t>(std::ceil(turns * static_cast<float>(kSegmentsPerTurn))), 1,
        kSegmentsPerTurn);

    // 로컬 좌표(중심 = {0, 0})의 꼭짓점을 [중심, 호 위의 점 segments + 1개, 중심] 순서로 둔다.
    // 면을 채우는 데는 앞쪽 segments + 2개를, 외곽선을 그릴 때는 뒤쪽 segments + 2개를 사용한다. (시작점만 다른 동일한 도형)
    std::array<Vec2f, kSegmentsPerTurn + 4> local{};
    const float start = detail::toRadians(detail::wrapDegrees(startDeg));
    const float step = detail::toRadians(sweepDeg) / static_cast<float>(segments);
    for (std::size_t i = 0; i <= segments; ++ i) {
        const float angle = start + step * static_cast<float>(i);
        local[i + 1] = Vec2f{std::cos(angle), std::sin(angle)} * radius;
    }

    // 면을 채울 때와 외곽선을 그을 때 모두 같은 변환을 사용하며, 순서는 sf::Shape와 같다.
    sf::Transformable placement;
    placement.setOrigin(backend::toSf(params.origin));
    placement.setRotation(backend::toSfAngle(params.rotation));
    placement.setPosition(backend::toSf(center));
    sf::RenderStates states;
    states.transform = placement.getTransform();

    // 180도가 넘는 부채꼴은 오목해서, 볼록한 도형만 보장하는 sf::Shape의 채우기를 쓸 수 없다.
    // 그러므로 실제 중심을 첫 점으로 하여 삼각형 형태의 부채를 펼친다.
    // 채우기 색이 투명하다면 GL 그리기 호출 수를 줄이기 위해 이 작업을 건너뛴다.
    if (params.fill.a != 0) {
        std::array<sf::Vertex, kSegmentsPerTurn + 2> fan{};
        const sf::Color fill = backend::toSf((params.fill));
        for (std::size_t i = 0; i < segments + 2; ++i) {
            fan[i] = sf::Vertex{backend::toSf(local[i]), fill};
        }
        impl_->target->draw(fan.data(), segments + 2, sf::PrimitiveType::TriangleFan, states);
    }

    // 외곽선은 있을 때만 sf::Shape로 만든다.
    // sf::Shape는 바운딩 박스의 중심과 첫 변으로 점의 방향을 정하므로 호부터 넘긴다.
    if (params.outlineThickness != 0.f) {
        PolygonShape outline{std::span<const Vec2f>{local}.subspan(1, segments + 3)};
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(backend::toSf(params.outline));
        outline.setOutlineThickness(params.outlineThickness);
        impl_->target->draw(outline, states);
    }
}

// --- 텍스트 ---------------------------------------------------------------

void Renderer::drawText(const Font& font, std::string_view text, Vec2f position,
                        const TextParams& params) {
    const sf::Font* native = backend::FontAccess::native(font);
    if (native == nullptr) {
        return;
    }

    sf::Text drawable = makeText(*native, text, params);
    drawable.setPosition(backend::toSf(position));

    impl_->target->draw(drawable);
}

Vec2f Renderer::measureText(const Font& font, std::string_view text,
                            const TextParams& params) const {
    return measureTextBounds(font, text, params).size;
}

Rectf Renderer::measureTextBounds(const Font& font, std::string_view text,
                                  const TextParams& params) const {
    const sf::Font* native = backend::FontAccess::native(font);
    if (native == nullptr) {
        return {};
    }

    // getLocalBounds()는 origin/position/rotation을 적용하기 전의 로컬 좌표를 반환한다.
    // SFML은 외곽선이 있으면 이 영역을 축마다 ceil(|t|)씩 양쪽으로 넓힌다.
    return backend::fromSf(makeText(*native, text, params).getLocalBounds());
}

// --- 카메라 ---------------------------------------------------------------
// 계산은 math/Camera2D.hpp의 자유 함수가 한다. 그리기(toSfView)도 같은 함수를 쓰므로 둘은 항상 일치한다.
// 자유 함수는 이 클래스의 멤버와 이름이 같아서, 멤버 안에서 그냥 부르면 멤버 자신이 불린다. 반드시 meno::를 붙인다.

void Renderer::setCamera(const Camera2D& camera) {
    impl_->camera = camera;
    impl_->applyView();
}

void Renderer::resetCamera() {
    impl_->camera.reset();
    impl_->applyView();
}

std::optional<Camera2D> Renderer::camera() const {
    return impl_->camera;
}

Vec2f Renderer::worldToScreen(Vec2f worldPoint) const {
    return meno::worldToScreen(impl_->activeCamera(), impl_->framebuffer(), worldPoint);
}

Vec2f Renderer::screenToWorld(Vec2f screenPoint) const {
    return meno::screenToWorld(impl_->activeCamera(), impl_->framebuffer(), screenPoint);
}

Rectf Renderer::visibleWorldBounds() const {
    return meno::visibleWorldBounds(impl_->activeCamera(), impl_->framebuffer());
}

// --- 정보 -----------------------------------------------------------------

Vec2u Renderer::framebufferSize() const {
    return impl_->framebuffer();
}

} // namespace meno
