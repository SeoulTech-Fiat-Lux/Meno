#include <meno/graphics/Texture.hpp>

#include "sfml_backend/Access.hpp"
#include "sfml_backend/Convert.hpp"

#include <SFML/Graphics/Image.hpp>

#include <utility>

namespace meno {

struct Texture::Impl {
    sf::Texture texture;
    bool loaded{false};
};

Texture::Texture() : impl_(std::make_unique<Impl>()) {}

Texture::~Texture() = default;

Texture::Texture(Texture&&) noexcept = default;

Texture& Texture::operator=(Texture&&) noexcept = default;

std::optional<Texture> Texture::loadFromFile(const std::filesystem::path& path) {
    Texture result;
    if (!result.impl_->texture.loadFromFile(path)) {
        return std::nullopt;
    }
    result.impl_->loaded = true;
    return result;
}

std::optional<Texture> Texture::fromPixels(Vec2u size, const std::uint8_t* rgba) {
    if (rgba == nullptr || size.x == 0 || size.y == 0) {
        return std::nullopt;
    }

    const sf::Image image{backend::toSf(size), rgba};

    Texture result;
    if (!result.impl_->texture.loadFromImage(image)) {
        return std::nullopt;
    }
    result.impl_->loaded = true;
    return result;
}

// 아래 세 함수는 이동 후 빈 껍데기(impl_ == nullptr)에서도 불릴 수 있다.
// 이동당한 텍스처는 크래시 없이 "빈 텍스처"처럼 행동한다는 것이 계약이다
// draw는 TextureAccess::native가 지켜 주지만 이 함수들은 native를 거치지 않으므로 직접 확인한다.
Vec2u Texture::size() const {
    if (impl_ == nullptr) {
        return {};
    }
    return backend::fromSf(impl_->texture.getSize());
}

void Texture::setSmooth(bool smooth) {
    if (impl_ == nullptr) {
        return;
    }
    impl_->texture.setSmooth(smooth);
}

bool Texture::isSmooth() const {
    return impl_ != nullptr && impl_->texture.isSmooth();
}

namespace backend {

const sf::Texture* TextureAccess::native(const Texture& texture) noexcept {
    // 이동 후 빈 껍데기(impl_ == nullptr)일 수 있으므로 둘 다 확인한다.
    if (texture.impl_ == nullptr || !texture.impl_->loaded) {
        return nullptr;
    }
    return &texture.impl_->texture;
}

} // namespace backend

} // namespace meno
