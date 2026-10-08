//
// Created by 최상준 on 26. 9. 7..
//

#include <meno/scene/Sprite.hpp>

#include <meno/scene/GameObject.hpp>
#include <meno/graphics/Renderer.hpp>

#include <cmath>
#include <numbers>

namespace meno {

SpriteParams Sprite::drawParams() const {
    SpriteParams params;
    params.position = offset;
    if (parent != nullptr) {
        const auto& transform = parent->transform();
        params.position = transform.pos;
        params.rotation = static_cast<float>(transform.rotation);
        const float scale = static_cast<float>(transform.magnitude);
        params.scale = {scale, scale};
        const float radians = params.rotation * std::numbers::pi_v<float> / 180.f;
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        params.position += Vec2f{offset.x * cosine - offset.y * sine,
                                 offset.x * sine + offset.y * cosine} * scale;
    }
    params.origin = origin;
    params.tint = tint;
    params.source = source;
    params.flipX = flipX;
    params.flipY = flipY;
    return params;
}

void Sprite::draw(Renderer& renderer) const {
    if (texture_) {
        renderer.draw(*texture_, drawParams());
    }
}

} // namespace meno
