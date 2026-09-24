#ifndef MENO_FONT_HPP
#define MENO_FONT_HPP

#include <filesystem>
#include <memory>
#include <optional>

namespace meno {

namespace backend {
struct FontAccess;
}

// 텍스트 렌더링용 폰트. Texture와 같은 소유권 규칙을 따른다.

// 이동된 Font로 drawText를 호출하면 아무것도 그리지 않고, 크기를 측정하면 {0, 0}을 반환한다.
// 공개 맴버 함수가 없어서 내부 핸들을 꺼내는 backend::FontAccess::native()를 거쳐서 이동한 후에 상태를 걸러야 한다.

// 한글을 그리려면 한글 글리프를 가진 폰트를 넣어야 한다.
// (Windows 기본 경로 예: C:/Windows/Fonts/malgun.ttf)
class Font {
public:
    Font();
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&&) noexcept;
    Font& operator=(Font&&) noexcept;

    // TTF/OTF 파일에서 로드. 실패하면 std::nullopt.
    [[nodiscard]] static std::optional<Font> loadFromFile(const std::filesystem::path& path);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    friend struct backend::FontAccess;
};

} // namespace meno

#endif  // MENO_FONT_HPP