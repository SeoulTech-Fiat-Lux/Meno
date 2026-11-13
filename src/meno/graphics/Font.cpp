#include <meno/graphics/Font.hpp>

#include "sfml_backend/Access.hpp"

namespace meno {

struct Font::Impl {
    sf::Font font;
    bool loaded{false};
};

Font::Font() : impl_(std::make_unique<Impl>()) {}

Font::~Font() = default;

Font::Font(Font&&) noexcept = default;

Font& Font::operator=(Font&&) noexcept = default;

std::optional<Font> Font::loadFromFile(const std::filesystem::path& path) {
    Font result;
    // SFML 3에서 Font의 로딩 함수는 loadFromFile이 아니라 openFromFile이다.
    // (폰트는 파일 핸들을 계속 열어둔 채로 글리프를 요청받아 캐싱한다.)
    if (!result.impl_->font.openFromFile(path)) {
        return std::nullopt;
    }
    result.impl_->loaded = true;
    return result;
}

namespace backend {

const sf::Font* FontAccess::native(const Font& font) noexcept {
    if (font.impl_ == nullptr || !font.impl_->loaded) {
        return nullptr;
    }
    return &font.impl_->font;
}

} // namespace backend

} // namespace meno
