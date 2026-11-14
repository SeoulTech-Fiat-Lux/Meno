#ifndef MENO_WINDOW_HPP
#define MENO_WINDOW_HPP

#include <meno/math/Vec2.hpp>

#include <memory>
#include <string_view>

namespace meno {

/// \addtogroup window
/// \{

class Input;
namespace backend {
struct WindowAccess;
}

// Renderer가 생성 시점에 이 창의 렌더 타깃 주소를 붙잡으므로 복사도 이동도 막는다.
/// OS 창 하나와 그 창의 입력 상태를 소유합니다.
///
/// Game을 쓰면 Game이 창을 만들고, 프레임마다 pollEvents()를 호출하며, 창이 닫히면 루프를
/// 멈춥니다. Game 없이 쓸 때는 아래와 같이 루프를 직접 실행합니다.
///
/// 복사도 이동도 할 수 없습니다. 생성을 미뤄야 하면 std::optional<Window>::emplace나
/// std::unique_ptr<Window>를 씁니다. 이 창으로 만든 Renderer보다 창이 오래 살아 있어야 합니다.
///
/// \code{.cpp}
/// meno::Window window{{800, 600}, "meno"};
/// meno::Renderer renderer{window};
/// while (window.isOpen()) {
///     window.pollEvents();
///     renderer.beginFrame();
///     // 그리기
///     renderer.endFrame();
/// }
/// \endcode
///
/// \see Game::window, Renderer, Input
class Window {
public:
    /// size 픽셀 크기의 창을 만들어 띄웁니다. title은 UTF-8로 해석됩니다.
    Window(Vec2u size, std::string_view title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    /// 창이 열려 있으면 true입니다. 닫기 버튼을 누르면 pollEvents()가 창을 닫습니다.
    [[nodiscard]] bool isOpen() const;
    /// 창을 닫습니다. 이후 isOpen()은 false입니다.
    void close();

    /// 쌓인 OS 이벤트를 모두 처리합니다. 프레임마다 Renderer::beginFrame()보다 먼저 호출합니다.
    ///
    /// 닫기 버튼을 누르면 창을 닫고, 키보드·마우스 이벤트는 input()에 넘깁니다. 창 크기 변경도
    /// 이때 반영되므로, beginFrame()보다 먼저 호출해야 새 크기로 그립니다.
    void pollEvents();

    /// 이 창으로 들어온 키보드·마우스 입력입니다. pollEvents()가 갱신합니다.
    /// 반환한 참조는 창이 살아 있는 동안 유효합니다.
    /// \{
    [[nodiscard]] Input& input();
    [[nodiscard]] const Input& input() const;
    /// \}

    /// 창 안쪽(그리기 영역)의 픽셀 크기입니다. 사용자가 창 크기를 바꾸면 pollEvents() 뒤에
    /// 새 크기를 돌려줍니다.
    [[nodiscard]] Vec2u size() const;

private:
    struct Impl;
    // SFML 타입을 공개 헤더에서 감추는 pimpl.
    // 생성자에서 만들고 이동이 없으므로 끝까지 nullptr이 아니다.
    std::unique_ptr<Impl> impl_;

    friend struct backend::WindowAccess;
};

/// \}

} // namespace meno

#endif // MENO_WINDOW_HPP
