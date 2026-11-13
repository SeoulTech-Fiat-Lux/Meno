#ifndef MENO_WINDOW_HPP
#define MENO_WINDOW_HPP

#include <meno/math/Vec2.hpp>

#include <memory>
#include <string_view>

namespace meno {

class Input;
namespace backend {
struct WindowAccess;
}

/// OS 창 하나와 그 창의 입력 상태를 소유한다.
///
/// Renderer가 생성 시점에 이 창의 렌더 타깃 주소를 붙잡으므로, Window는 복사도 이동도 할 수 없다.
/// 생성을 미뤄야 하면 std::optional<Window>::emplace나 std::unique_ptr<Window>를 쓴다.
class Window {
public:
    Window(Vec2u size, std::string_view title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    [[nodiscard]] bool isOpen() const;
    void close();

    /// OS 이벤트를 처리한다. 닫기 버튼은 창을 닫고, 입력 이벤트는 Input으로 넘긴다.
    /// 창 크기 변경은 여기서 이벤트를 꺼낼 때 SFML이 반영한다. Renderer::beginFrame보다 먼저 부를
    /// 것.
    void pollEvents();

    /// 이 창으로 들어온 키보드·마우스 입력. pollEvents()가 갱신한다.
    /// 반환한 참조는 창이 살아 있는 동안 유효하다.
    [[nodiscard]] Input& input();
    [[nodiscard]] const Input& input() const;

    [[nodiscard]] Vec2u size() const;

private:
    struct Impl;
    // SFML 타입을 공개 헤더에서 감추는 pimpl.
    // 생성자에서 만들고 이동이 없으므로 끝까지 nullptr이 아니다.
    std::unique_ptr<Impl> impl_;

    friend struct backend::WindowAccess;
};

} // namespace meno

#endif // MENO_WINDOW_HPP
