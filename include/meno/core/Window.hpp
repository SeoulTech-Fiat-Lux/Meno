#pragma once

#include <meno/math/Vec2.hpp>

#include <memory>
#include <string_view>

namespace meno {

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

    /// OS 이벤트를 처리한다. 지금은 닫기 버튼만 다룬다.
    /// 실제 입력 처리는 Input 서비스 담당자가 가져갈 부분.
    void pollEvents();

    [[nodiscard]] Vec2u size() const;

    // 0이면 무제한. 실제 시간 관리는 Clock이 담당할 것이므로 임시로 배치한 함수.
    // 실제 Clock의 구현에 따라 변경할 수 있다.
    void setFramerateLimit(unsigned int fps);

private:
    struct Impl;
    // Pimpl(pointer to implementation)패턴을 사용하여 구현 세부 사항을 숨김. Window의 실제 구현은 Impl 구조체에 존재.
    // 현 파일 기준으로 Window 객체는 Impl 구조체를 unique_ptr로 소유한다.
    // Window 객체가 소멸될 때, Impl -> sf::Window 순으로 소멸하며 GPU 메모리가 해제된다.
    // 위 사안을 new/delete를 직접 호출하지 않고 unique_ptr가 자동으로 관리.
    std::unique_ptr<Impl> impl_;

    friend struct backend::WindowAccess;
}

} // namespace meno