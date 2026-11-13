#ifndef MENO_FONT_HPP
#define MENO_FONT_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

/// \addtogroup graphics
/// \{

namespace backend {
struct FontAccess;
}

/// 텍스트를 그릴 때 쓰는 폰트입니다.
///
/// 복사할 수 없고 이동할 수 있습니다. 로드하지 않았거나 이동된 Font로 drawText()를 부르면 아무것도
/// 그리지 않고, measureText()는 {0, 0}을 돌려줍니다.
///
/// 주의할 점:
/// - 폰트 파일은 Font가 살아 있는 동안 계속 열려 있으므로, 그동안 파일을 옮기거나 지우면 안 됩니다.
/// - 한글을 그리려면 한글 글리프가 있는 폰트를 써야 합니다. 예: C:/Windows/Fonts/malgun.ttf
///
/// \code{.cpp}
/// const std::optional<meno::Font> font = meno::Font::loadFromFile("assets/fonts/game.ttf");
/// if (font) {
///     renderer.drawText(*font, "점수: 100", {16.f, 16.f}, {.characterSize = 32});
/// }
/// \endcode
///
/// \see Renderer::drawText, TextParams, ResourceManager::loadFont
class Font {
public:
    /// 빈 폰트를 만듭니다. 이 폰트로는 아무것도 그려지지 않습니다.
    Font();
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    /// 이동합니다. 이동된 쪽은 빈 폰트처럼 동작합니다.
    /// \{
    Font(Font&&) noexcept;
    Font& operator=(Font&&) noexcept;
    /// \}

    /// TTF/OTF 파일을 엽니다. 실패하면 std::nullopt를 돌려줍니다.
    [[nodiscard]] static std::optional<Font> loadFromFile(const std::filesystem::path& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend struct backend::FontAccess;
};

/// \}

} // namespace meno

#endif // MENO_FONT_HPP
