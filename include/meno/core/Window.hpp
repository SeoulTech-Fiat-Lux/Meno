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

// ⚠️ Core 계층 담당자가 갱신해야 할 영역.
//
// 현재 구축된 렌더 API가 정상 구동되는지 확인하기 위한 최소 스텁.
// Core 담당자가 실제 Window(이벤트 큐, Clock, Config 연동)를 만들면 이 파일은 통째로 교체되어야 한다.
//
// *Renderer가 Window에게 요구하는 사안
// 1. backend::WindowAccess로 꺼낼 수 있는 렌더 타깃
// 2. framebuffer 크기
// 이 두 가지만 유지되면 Core 쪽이 어떻게 바뀌든 Renderer는 영향받지 X.
class Window {
public:
    // 생성자: 윈도우 크기와 제목을 받아 객체를 초기화.
    Window(Vec2u size, std::string_view title);
    // 소멸자: Window 객체가 소멸될 때 리소스를 해제.
    ~Window();

    // 복사 금지
    // 복사 생성자와 복사 대입 연산자를 삭제하여 Window 객체의 복사를 방지.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // 이동 가능
    // 이동 생성자와 이동 대입 연산자를 정의하여 Window 객체의 이동을 허용.
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;

    [[nodiscard]] bool isOpen() const;
    void close();

    /// OS 이벤트를 처리한다. 닫기 버튼은 창을 닫고, 입력 이벤트는 Input으로 넘긴다.
    void pollEvents();

    // 이 창으로 들어온 키보드 및 마우스 입력을 관리하는 부분.
    // pollEvents()가 갱신하며, 반환한 참조는 창과 수명이 동일하다.
    // 이동 생성한 새 창으로 그대로 따라가나, 다른 창을 이동 대입받는다면 기존 참조는 무효가 된다.
    // 혹시 이동당한 Window에서 호출한다면, std::logic_error를 던진다.
    [[nodiscard]] Input& input();
    [[nodiscard]] const Input& input() const;

    [[nodiscard]] Vec2u size() const;

private:
    struct Impl;
    // Pimpl(pointer to implementation)패턴을 사용하여 구현 세부 사항을 숨김. Window의 실제 구현은
    // Impl 구조체에 존재. 현 파일 기준으로 Window 객체는 Impl 구조체를 unique_ptr로 소유한다.
    // Window 객체가 소멸될 때, Impl -> sf::Window 순으로 소멸하며 GPU 메모리가 해제된다.
    // 위 사안을 new/delete를 직접 호출하지 않고 unique_ptr가 자동으로 관리.
    std::unique_ptr<Impl> impl_;

    friend struct backend::WindowAccess;
};

} // namespace meno

#endif  // MENO_WINDOW_HPP