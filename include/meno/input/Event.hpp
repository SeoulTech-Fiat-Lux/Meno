#ifndef MENO_EVENT_HPP
#define MENO_EVENT_HPP

#include <meno/input/Key.hpp>
#include <meno/input/MouseButton.hpp>
#include <meno/math/Vec2.hpp>

#include <variant>

namespace meno {

// SFML 없이 표현한 입력 이벤트.
// Window가 SFML 이벤트를 Input에 넘길 때, 이 형태로 변환한다.
// 테스트나 입력 재생과 같이, 창 없이 Input을 구동할 때는 직접 만들어서 Input::handleEvent()에 넘긴다.

// ex. input.handleEvent(meno::event::KeyPressed{meno::Key::Space});

// 현재 모든 좌표의 기준은 창의 좌상단 픽셀이다.
namespace event {

// 창이 포커스를 잃는다면, Input은 모든 키와 마우스 버튼을 "떼어진 상태"로 처리한다.
struct FocusLost {};

// 키를 눌렀을 때 발생한다. 키를 누르고 있으면 OS의 키 반복 기능에 따라 중복 발생하여 꾹 누르는 입력을 처리할 수 있다.
struct KeyPressed {
    Key key = Key::Unknown;
};

struct KeyReleased {
    Key key = Key::Unknown;
};

struct MouseButtonPressed {
    MouseButton button = MouseButton::Left;
    Vec2i position;
};

struct MouseButtonReleased {
    MouseButton button = MouseButton::Left;
    Vec2i position;
};

struct MouseMoved {
    Vec2i position;
};

// 세로 휠. 위로 굴리면 양수, 아래로 굴리면 음수로 처리한다. 정밀 휠은 소수점 단위로 값을 전달한다.
struct MouseWheelScrolled {
    float delta = 0.f;
    Vec2i position;
};

// 입력된 문자 하나를 UTF-32 코드로 전달한다. IME는 조합이 끝난 글자만 보내므로, 조합 중인 글자는 전달되지 않는다.
struct TextEntered {
    char32_t unicode = 0;
};

} // namespace event

using Event = std::variant<event::FocusLost, event::KeyPressed, event::KeyReleased,
                            event::MouseButtonPressed, event::MouseButtonReleased, event::MouseMoved,
                            event::MouseWheelScrolled, event::TextEntered>;

} // namespace meno
#endif // MENO_EVENT_HPP