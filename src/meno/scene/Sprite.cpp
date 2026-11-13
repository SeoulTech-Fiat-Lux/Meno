#include <meno/scene/Sprite.hpp>

#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>
#include <meno/math/Angle.hpp>
#include <meno/scene/GameObject.hpp>

namespace meno {

void Sprite::setTexture(Texture&& texture) {
    texture_ = std::make_shared<Texture>(std::move(texture));
}

SpriteParams Sprite::drawParams() const {
    SpriteParams params;
    params.position = offset;
    if (parent() != nullptr) {
        const auto& transform = parent()->transform();
        params.position = transform.transformPoint(offset);
        params.rotation = wrapDegrees(transform.rotation);
        const float scale = transform.scale;
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
