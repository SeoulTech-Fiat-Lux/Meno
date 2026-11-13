#ifndef MENO_EVENT_HPP
#define MENO_EVENT_HPP

#include <meno/input/Key.hpp>
#include <meno/input/MouseButton.hpp>
#include <meno/math/Vec2.hpp>

#include <variant>

namespace meno {

// SFML 없이 표현한 입력 이벤트다. Window가 SFML 이벤트를 Input에 넘길 때 이 형태로 바꾼다.
/// Input에 넣는 입력 이벤트입니다.
///
/// 보통은 Window::pollEvents()가 만들어 넣으므로 직접 다룰 일이 없습니다. 테스트나 입력 재생처럼
/// 창 없이 Input을 구동할 때는 직접 만들어 Input::handleEvent()에 넘깁니다. 좌표는 모두 창 좌상단
/// 기준의 픽셀 좌표입니다.
///
/// \code{.cpp}
/// input.beginFrame();
/// input.handleEvent(meno::event::KeyPressed{meno::Key::Space});
/// \endcode
namespace event {

/// 창이 포커스를 잃었습니다. Input은 눌려 있던 키와 마우스 버튼을 모두 떼어진 것으로 처리합니다.
struct FocusLost {};

/// 키를 눌렀습니다.
///
/// 누르고 있으면 OS의 키 반복으로 여러 번 오지만 Input은 반복을 무시합니다. 키를 누르고 있는지는
/// Input::isDown()으로 확인합니다.
struct KeyPressed {
    /// 누른 키입니다. Key에 없는 키는 Key::Unknown이며, Input은 Unknown을 추적하지 않습니다.
    Key key = Key::Unknown;
};

/// 키를 뗐습니다.
struct KeyReleased {
    Key key = Key::Unknown; ///< 뗀 키
};

/// 마우스 버튼을 눌렀습니다.
struct MouseButtonPressed {
    MouseButton button = MouseButton::Left; ///< 누른 버튼
    Vec2i position;                         ///< 누른 위치
};

/// 마우스 버튼을 뗐습니다.
struct MouseButtonReleased {
    MouseButton button = MouseButton::Left; ///< 뗀 버튼
    Vec2i position;                         ///< 뗀 위치
};

/// 마우스가 움직였습니다.
struct MouseMoved {
    Vec2i position; ///< 새 위치
};

/// 세로 휠을 굴렸습니다. 가로 휠은 오지 않습니다.
struct MouseWheelScrolled {
    /// 굴린 양입니다. 위로 굴리면 양수, 아래로 굴리면 음수이며, 정밀 휠은 소수점 값을 보냅니다.
    float delta = 0.f;
    Vec2i position; ///< 굴린 위치
};

/// 문자 하나가 입력되었습니다. IME는 조합이 끝난 글자만 보내므로, 조합 중인 글자는 오지 않습니다.
struct TextEntered {
    char32_t unicode = 0; ///< 입력된 문자의 UTF-32 코드
};

} // namespace event

/// Input::handleEvent()가 받는 이벤트 하나입니다. event 네임스페이스의 타입 중 하나를 담습니다.
using Event = std::variant<event::FocusLost, event::KeyPressed, event::KeyReleased,
                           event::MouseButtonPressed, event::MouseButtonReleased, event::MouseMoved,
                           event::MouseWheelScrolled, event::TextEntered>;

} // namespace meno
#endif // MENO_EVENT_HPP
