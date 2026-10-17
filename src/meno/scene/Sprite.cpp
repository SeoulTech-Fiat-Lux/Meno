//
// Created by 최상준 on 26. 9. 7..
//

#include <meno/scene/Sprite.hpp>

#include <meno/scene/GameObject.hpp>
#include <meno/graphics/Renderer.hpp>

namespace meno {

SpriteParams Sprite::drawParams() const {
    SpriteParams params;
    params.position = offset;
    if (parent != nullptr) {
        const auto& transform = parent->transform();
        params.position = transform.transformPoint(offset);
        params.rotation = static_cast<float>(transform.rotation % 360);
        const float scale = transform.magnitude;
        params.scale = {scale, scale};
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
