#ifndef MENO_INPUT_HPP
#define MENO_INPUT_HPP

#include <meno/input/Event.hpp>
#include <meno/input/Key.hpp>
#include <meno/input/MouseButton.hpp>
#include <meno/math/Vec2.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace meno {

/// \addtogroup input
/// \{

/// 키보드와 마우스의 입력 상태입니다.
///
/// 보통은 Window가 Input 하나를 갖고 있고 Window::pollEvents()가 채우므로, window.input()으로
/// 읽기만 합니다. 키를 누르고 있는지는 isDown()으로, 누른 순간은 wasPressed()로 확인합니다.
///
/// "이번 틱" 규칙: wasPressed(), wasReleased(), mouseWheelDelta(), text(), wasActionPressed(),
/// wasActionReleased()는 이번 틱에 일어난 일만 돌려줍니다. 틱은 읽는 곳에 따라 다릅니다.
/// - Application::update() 안: 그 update() 한 번입니다. 한 프레임의 update() 실행 횟수가 0번이든
///   여러 번이든, 한 번 누른 키는 그 뒤 처음 실행되는 update()에서만 wasPressed()가 true입니다.
/// - 그 밖(render(), 직접 만든 루프 등): pollEvents()부터 다음 pollEvents()까지입니다.
///
/// 창이 포커스를 잃으면 눌려 있던 키와 버튼은 모두 떼어진 것으로 처리됩니다.
///
/// \code{.cpp}
/// const meno::Input& input = window.input();
/// if (input.wasPressed(meno::Key::Space)) {
///     jump();
/// }
/// if (input.isDown(meno::Key::Right)) {
///     moveRight();
/// }
/// \endcode
///
/// \see Window::input, Key, MouseButton
class Input {
public:
    /// 지금 키나 마우스 버튼이 눌려 있으면 true입니다.
    /// \{
    [[nodiscard]] bool isDown(Key key) const noexcept;
    [[nodiscard]] bool isDown(MouseButton button) const noexcept;
    /// \}

    /// 이번 틱에 키나 마우스 버튼을 눌렀으면 true입니다. 같은 틱 안에 눌렀다 떼도 true입니다.
    ///
    /// OS의 키 반복은 무시되므로, 누르고 있는 동안 다시 true가 되지 않습니다.
    /// \{
    [[nodiscard]] bool wasPressed(Key key) const noexcept;
    [[nodiscard]] bool wasPressed(MouseButton button) const noexcept;
    /// \}

    /// 이번 틱에 키나 마우스 버튼을 뗐으면 true입니다. 창이 포커스를 잃어 떼어진 것으로 처리된
    /// 경우도 포함됩니다.
    /// \{
    [[nodiscard]] bool wasReleased(Key key) const noexcept;
    [[nodiscard]] bool wasReleased(MouseButton button) const noexcept;
    /// \}

    /// 마지막으로 알려진 마우스 위치입니다. 아직 마우스 이벤트가 없으면 {0, 0}입니다.
    ///
    /// 창 좌상단 기준의 픽셀 좌표로, 카메라를 적용하지 않은 값입니다. 월드 좌표가 필요하면
    /// renderer.screenToWorld(Vec2f(input.mousePosition()))로 변환합니다.
    [[nodiscard]] Vec2i mousePosition() const noexcept;
    /// 이번 틱에 굴린 세로 휠 양의 합입니다. 위로 굴리면 양수입니다.
    [[nodiscard]] float mouseWheelDelta() const noexcept;

    /// 이번 틱에 입력된 문자를 UTF-8로 돌려줍니다.
    ///
    /// 백스페이스, 엔터, 탭과 같은 제어 문자는 빠지므로 wasPressed(Key::Backspace)처럼 따로
    /// 처리합니다. 값으로 돌려주기 때문에 보관해도 됩니다.
    [[nodiscard]] std::string text() const;

    /// "jump"와 같은 동작 이름에 키나 마우스 버튼을 묶습니다. 한 동작에 여러 개를 묶을 수 있으며,
    /// 하나라도 눌려 있으면 동작이 눌린 것으로 봅니다.
    ///
    /// \code{.cpp}
    /// input.bind("jump", meno::Key::Space);
    /// input.bind("jump", meno::Key::W);
    /// if (input.wasActionPressed("jump")) {
    ///     jump();
    /// }
    /// \endcode
    ///
    /// 같은 키를 두 번 묶어도 한 번만 묶입니다. 이미 눌려 있는 키를 묶으면 isActionDown()은 바로
    /// true지만, 이번 틱에 누른 것은 아니므로 wasActionPressed()는 false입니다.
    /// \{
    void bind(std::string_view action, Key key);
    void bind(std::string_view action, MouseButton button);
    /// \}

    /// 동작을 지우고 묶인 키와 버튼을 모두 풉니다. 등록하지 않은 이름이면 아무 일도 하지 않습니다.
    ///
    /// 키 하나만 풀 수는 없으므로, 묶인 키를 바꾸려면 unbind()한 뒤에 다시 bind()합니다.
    void unbind(std::string_view action);

    /// 동작에 묶인 키나 버튼이 하나라도 눌려 있으면 true입니다. 등록하지 않은 이름이면 false입니다.
    [[nodiscard]] bool isActionDown(std::string_view action) const;
    /// 동작이 눌리지 않은 상태에서 눌린 상태로 바뀐 틱에만 true입니다. 등록하지 않은 이름이면
    /// false입니다.
    ///
    /// 예를 들어 Space와 W를 묶은 동작에서 Space를 누른 채 W를 더 눌러도 true가 되지 않습니다.
    [[nodiscard]] bool wasActionPressed(std::string_view action) const;
    /// 동작에 묶인 키와 버튼이 모두 떼어진 틱에만 true입니다. 창이 포커스를 잃어 떼어진 것으로
    /// 처리된 경우도 포함됩니다. 등록하지 않은 이름이면 false입니다.
    [[nodiscard]] bool wasActionReleased(std::string_view action) const;

    /// 새 프레임을 시작합니다. 이벤트를 넣기 전에 프레임마다 한 번 호출합니다.
    ///
    /// Window::pollEvents()가 대신 호출합니다. 직접 호출하는 경우는 테스트나 입력 재생처럼 창 없이
    /// Input을 구동할 때입니다.
    void beginFrame() noexcept;

    /// 이벤트 하나를 입력 상태에 반영합니다.
    ///
    /// Window::pollEvents()가 대신 호출합니다. 직접 호출하는 경우는 테스트나 입력 재생처럼 창 없이
    /// Input을 구동할 때입니다.
    void handleEvent(const Event& event);

private:
    static constexpr std::uint64_t never = std::numeric_limits<std::uint64_t>::max();

    // 이벤트가 처리된 시점으로, 통칭 틱(Tick).
    // 프레임 번호와 해당 이벤트를 처음 보게 될 update()의 단계 번호를 함께 기록한다.
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
    template <typename T>
    struct Accumulator {
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

    // 액션의 상태는 매번 계산하지 않은 채로 들고 있다.
    // 묶인 키나 버튼이 안 눌려있다가 눌리는 순간을 동작 단위로 적어야 하기 때문이다.
    struct Action {
        std::vector<Key> keys;
        std::vector<MouseButton> buttons;
        bool down = false;
        Tick pressed;
        Tick released;
    };

    [[nodiscard]] Tick now() const noexcept;
    [[nodiscard]] bool isCurrent(const Tick& tick) const noexcept;
    template <typename T>
    [[nodiscard]] T current(const Accumulator<T>& accumulator) const;

    // 이미 눌린 키의 자동 반복은 무시하는 함수로, 상태가 바뀌면 true를 반환한다.
    static bool press(ButtonState& state, const Tick& now) noexcept;
    static bool release(ButtonState& state, const Tick& now) noexcept;

    // press 또는 release를 가리키는 함수 포인터
    using ButtonChange = bool (*)(ButtonState&, const Tick&) noexcept;
    // 키나 버튼 하나에 change를 적용하고, 상태가 실제로 바뀐 경우에만 액션을 다시 계산한다.
    // state가 nullptr이라면 아무 것도 수행하지 않는다.
    void applyButton(ButtonState* state, ButtonChange change) noexcept;

    [[nodiscard]] const Action* findAction(std::string_view name) const;
    [[nodiscard]] bool anyBoundDown(const Action& action) const noexcept;
    // 키나 버튼의 상태가 바뀐 뒤 호출한다.
    // 각 동작이 눌렸는지에 대한 여부를 다시 계산하고, 바뀐 액션에 대해서는 현재 시점을 적는다.
    void refreshActions(const Tick& now) noexcept;

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
    Vec2i mousePosition_;
    Accumulator<float> wheel_;
    Accumulator<std::string> text_;
    std::map<std::string, Action, std::less<>> actions_;
    std::uint64_t frame_ = 0;
};

/// \}

} // namespace meno

#endif // MENO_INPUT_HPP
