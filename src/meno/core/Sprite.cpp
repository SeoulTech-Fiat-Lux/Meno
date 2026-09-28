//
// Created by 최상준 on 26. 9. 7..
//

#include <meno/core/Sprite.hpp>

#include <meno/core/GameObject.hpp>
#include <meno/graphics/Renderer.hpp>

namespace meno {

SpriteParams Sprite::drawParams() const {
    SpriteParams params;
    if (parent != nullptr) {
        const auto& transform = parent->transform();
        params.position = transform.pos;
        params.rotation = static_cast<float>(transform.rotation);
        const float scale = static_cast<float>(transform.magnitude);
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
