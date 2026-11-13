#ifndef MENO_SPRITE_HPP
#define MENO_SPRITE_HPP

#include <meno/graphics/DrawParams.hpp>
#include <meno/scene/Component.hpp>

#include <memory>
#include <optional>
#include <utility>

namespace meno {

/// \addtogroup scene
/// \{

class Renderer;
class Texture;

/// GameObject의 그림입니다. 모든 GameObject가 기본으로 가지며, 텍스처가 있을 때만 그립니다.
///
/// 텍스처는 shared_ptr로 공유하므로 여러 GameObject가 같은 텍스처를 써도 한 번만 로드됩니다.
/// 파일은 ResourceManager::loadTexture()로 읽어 setTexture()에 넘깁니다. 위치, 회전, 배율은 부모
/// GameObject의 Transform을 따릅니다.
///
/// \code{.cpp}
/// player.sprite().setTexture(resources.loadTexture("assets/player.png"));
/// player.sprite().flipX = true; // 왼쪽을 보게 합니다.
/// \endcode
///
/// \see GameObject::sprite, SpriteParams
class Sprite : public Component {
private:
    std::shared_ptr<const Texture> texture_;

public:
    /// 스프라이트의 origin이 놓일 로컬 위치(GameObject 좌상단 기준)입니다. 부모 Transform의 배율과
    /// 회전을 따릅니다.
    Vec2f offset{0.f, 0.f};
    /// 텍스처 안의 기준점입니다. 기본값은 좌상단이며, GameObject의 원점은 바꾸지 않습니다.
    Vec2f origin{0.f, 0.f};
    /// 텍스처 색에 곱하는 색입니다. SpriteParams::tint와 같습니다.
    Color tint{colors::White};
    /// 텍스처의 일부만 그릴 때의 영역입니다. SpriteParams::source와 같습니다.
    std::optional<Recti> source;
    /// 좌우(flipX)나 상하(flipY)로 뒤집어 그립니다. SpriteParams::flipX와 같습니다.
    /// \{
    bool flipX{false};
    bool flipY{false};
    /// \}

public:
    /// 텍스처 없는 Sprite를 만듭니다.
    Sprite() = default;

    /// 텍스처를 넘겨받아 이 Sprite만 씁니다. 복사하지 않습니다.
    void setTexture(Texture&& texture);
    /// 텍스처를 공유합니다. nullptr을 넘기면 그리기를 멈추고 기존 텍스처를 놓습니다.
    void setTexture(std::shared_ptr<const Texture> texture) noexcept {
        texture_ = std::move(texture);
    }

    /// 지금 쓰는 텍스처입니다. 없으면 빈 shared_ptr입니다.
    [[nodiscard]] const std::shared_ptr<const Texture>& texture() const noexcept {
        return texture_;
    }

    /// 부모 Transform을 적용한 그리기 옵션입니다.
    /// 부모가 없으면 offset을 그대로 위치로 쓰고 회전과 배율은 기본값입니다.
    [[nodiscard]] SpriteParams drawParams() const;

    /// 텍스처를 그립니다. 텍스처가 없으면 아무것도 하지 않습니다. 보통은 Scene::drawScene()이
    /// 부릅니다.
    void draw(Renderer& renderer) const;
};

/// \}

} // namespace meno

#endif // MENO_SPRITE_HPP
