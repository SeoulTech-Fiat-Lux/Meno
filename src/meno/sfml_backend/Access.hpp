#ifndef MENO_ACCESS_HPP
#define MENO_ACCESS_HPP

#include <meno/core/Window.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Texture.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

/// 내부 전용 헤더. include/ 아래로 절대 옮기지 말 것.
///
/// pimpl로 감춘 SFML 객체를 백엔드 구현끼리만 꺼내 쓰기 위한 통로다.
/// 각 클래스가 이 struct들을 friend로 선언해 두어서, 여기를 거치지 않으면
/// 내부 SFML 핸들에 접근할 수 없다. 사용자 코드는 이 헤더를 볼 수 없으므로
/// 백엔드는 계속 감춰진 상태로 남는다.
namespace meno::backend {

struct TextureAccess {
    /// 로드되지 않은 Texture면 nullptr.
    [[nodiscard]] static const sf::Texture* native(const Texture& texture) noexcept;
};

struct FontAccess {
    [[nodiscard]] static const sf::Font* native(const Font& font) noexcept;
};

struct WindowAccess {
    [[nodiscard]] static sf::RenderWindow* native(const Window& window) noexcept;
};

} // namespace meno::backend

#endif  // MENO_ACCESS_HPP