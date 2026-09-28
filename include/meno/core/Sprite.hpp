//
// Created by 최상준 on 26. 9. 7..
//

#ifndef MENO_SPRITE_HPP
#define MENO_SPRITE_HPP

#include <meno/core/Component.hpp>
#include <meno/graphics/DrawParams.hpp>
#include <meno/graphics/Texture.hpp>
#include <meno/graphics/Renderer.hpp>

#include <memory>
#include <utility>


namespace meno {

class Renderer;
class Texture;

/// 로드된 텍스처를 공유한다. 파일 로딩과 경로별 캐싱은 호출자 담당이다.
class Sprite : public Component {
private:
    std::shared_ptr<const Texture> texture_;

public:
    Vec2f origin{0.f, 0.f};
    Color tint{colors::White};
    std::optional<Recti> source{};
    bool flipX{false};
    bool flipY{false};

public:
    explicit Sprite(GameObject* parent) { this->parent = parent; }

    /// 텍스처를 복사하지 않고 소유권을 넘겨받는다.
    void setTexture(Texture&& texture) {
        texture_ = std::make_shared<Texture>(std::move(texture));
    }
    /// nullptr을 지정하면 그리기를 중단하고 기존 텍스처 참조를 해제한다.
    void setTexture(std::shared_ptr<const Texture> texture) noexcept {
        texture_ = std::move(texture);
    }

    [[nodiscard]] const std::shared_ptr<const Texture>& texture() const noexcept { return texture_; }

    /// 위치/회전/배율은 부모 Transform에서 가져온다. width/height는 사용하지 않는다.
    /// 부모가 없으면 기본 변환을 사용한다.
    [[nodiscard]] SpriteParams drawParams() const;

    /// 파일 로딩 없이 기존 텍스처를 그린다. 텍스처가 없으면 건너뛴다.
    void draw(Renderer& renderer) const;
};

}


#endif // MENO_SPRITE_HPP
