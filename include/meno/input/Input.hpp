#ifndef MENO_INPUT_HPP
#define MENO_INPUT_HPP

#include <meno/input/Event.hpp>
#include <meno/input/Key.hpp>
#include <meno/input/MouseButton.hpp>
#include <meno/math/Vec2.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <string>

namespace meno {

// 키보드와 마우스의 입력 상태를 관리한다.
// 보통은 Window가 input 하나를 갖고 있고, window.pollEvent()가 채운다.

// ex. if (window.input().wasPressed(meno::Key::space)) { jump(); }

// wasPressed(), wasReleased()와 휠 및 텍스트는 "이번 틱"에 발생한 일만 반환한다. 틱은 호출된 곳에 따라 달라지며, 아래와 같이 정의한다.
// Application::update 내부: 해당 fixed update 한 번.
// 한 프레임에 update()가 0번 돌든 여러 번 돌든 관계 없이, 한 번 누른 키는 그 뒤에 처음 도는 update()에서만 wasPressed()가 true가 된다.
// 그 밖(render, 직접 구성한 루프 등): pollEvents부터 다음 pollEvents까지.
class Input {
public:
    // --- 키보드 ----------------------------------------------------
    // 지금 키가 눌린 상태인가
    [[nodiscard]] bool isDown(Key key) const noexcept;
    // 이번 틱에 키가 눌렸는가 (같은 틱 안에 눌렀다 떼도 true로 처리한다.)
    [[nodiscard]] bool wasPressed(Key key) const noexcept;
    // 이번 틱에 키가 떼졌는가
    [[nodiscard]] bool wasReleased(Key key) const noexcept;

    // --- 마우스 ----------------------------------------------------
    [[nodiscard]] bool isDown(MouseButton button) const noexcept;
    [[nodiscard]] bool wasPressed(MouseButton button) const noexcept;
    [[nodiscard]] bool wasReleased(MouseButton button) const noexcept;

    // 마지막으로 인식된 마우스 좌표. 창의 좌상단 픽셀을 기준으로 하며, 카메라를 고려하지 않는다.
    // 월드 좌표가 필요하다면, Renderer::screenToWorld()를 활용하여 변환한다.
    [[nodiscard]] Vec2i mousePosition() const noexcept;
    // 이번 틱에 굴린 마우스 휠의 세로 이동량 합계
    [[nodiscard]] float mouseWheelDelta() const noexcept;

    // --- 텍스트 ----------------------------------------------------
    // 이번 틱에 입력된 문자.
    // 백스페이스나 엔터 등의 제어 문자는 제외되므로 wasPressed(Key::Backspace)와 같이 키 입력 이벤트를 따로 처리해야 한다.
    // 복사본을 들고 있기 때문에 다음 틱까지 들고 있어도 안전하다.
    [[nodiscard]] std::string text() const;

    // --- 이벤트 처리 ------------------------------------------------
    // Window::pollEvent()가 호출한다. 직접 호출하는 경우는 테스트나 입력 재생 등, 창 없이 Input을 구동할 때이다.

    // 새 프레임을 시작한다. 이벤트를 넣기 전에 프레임마다 한 번 호출한다.
    void beginFrame() noexcept;
    void handleEvent(const Event& event);

private:
    static constexpr std::uint64_t never = std::numeric_limits<std::uint64_t>::max();

    // 이벤트가 처리된 시점으로, 통칭 틱(Tick).
    // 프레임 번호와 해당 이벤트를 처음 보게 될 update()의 stepNumber_를 함께 기록한다.
    // Tick 내부 번호를 묻는 곳(update 안팎)에 따라 둘 중 하나를 선택하여 비교한다.
    struct Tick {
        std::uint64_t frame = never;
        std::uint64_t step = never;
    };

    struct ButtonState {
        bool down = false;
        Tick pressed;
        Tick released;
    };

    // 하나의 틱이 이뤄질 동안에 쌓이는 휠 혹은 텍스트 값.
    // 프레임용과 update용을 따로 두며, 다른 틱의 값이 들어오면 그때 비운다.
    template <typename T> struct Accumulator {
        T frameValue{};
        T stepValue{};
        Tick tick;

        void add(const T& value, const Tick& now) {
            if (tick.frame != now.frame) {
                frameValue = T{};
                tick.frame = now.frame;
            }
            if (tick.step != now.step) {
                stepValue = T{};
                tick.step = now.step;
            }
            frameValue += value;
            stepValue += value;
        }
    };

    [[nodiscard]] Tick now() const noexcept;
    [[nodiscard]] bool isCurrent(const Tick& tick) const noexcept;
    template <typename T> [[nodiscard]] T current(const Accumulator<T>& accumulator) const;

    // 이미 눌린 키의 자동 반복은 무시하는 함수로, 상태가 바뀌면 true를 반환한다.
    static bool press(ButtonState& state, const Tick& now) noexcept;
    static bool release(ButtonState& state, const Tick& now) noexcept;

    void on(const event::FocusLost& event);
    void on(const event::KeyPressed& event);
    void on(const event::KeyReleased& event);
    void on(const event::MouseButtonPressed& event);
    void on(const event::MouseButtonReleased& event);
    void on(const event::MouseMoved& event);
    void on(const event::MouseWheelScrolled& event);
    void on(const event::TextEntered& event);

    std::array<ButtonState, keyCount> keys_{};
    std::array<ButtonState, mouseButtonCount> mouseButtons_{};
    Vec2i mousePosition_{};
    Accumulator<float> wheel_;
    Accumulator<std::string> text_;
    std::uint64_t frame_ = 0;
};

} // namespace meno

#endif // MENO_INPUT_HPP
