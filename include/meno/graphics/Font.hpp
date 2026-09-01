#ifndef MENO_FONT_HPP
#define MENO_FONT_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

namespace backend {
struct FontAccess;
}

/// 텍스트 렌더링용 폰트. Texture와 같은 소유권 규칙을 따른다.
///
/// 한글을 그리려면 한글 글리프를 가진 폰트를 넣어야 한다.
/// (Windows 기본 경로 예: C:/Windows/Fonts/malgun.ttf)
class Font {
public:
    Font();
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&&) noexcept;
    Font& operator=(Font&&) noexcept;

    /// TTF/OTF 파일에서 로드. 실패하면 std::nullopt.
    [[nodiscard]] static std::optional<Font> loadFromFile(const std::filesystem::path& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend struct backend::FontAccess;
};

} // namespace meno

#endif  // MENO_FONT_HPP