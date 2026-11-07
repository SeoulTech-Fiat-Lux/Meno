#include <meno/input/Input.hpp>

#include "core/TimeAccess.hpp"

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <variant>

namespace meno {

using detail::TimeAccess;

namespace {

// 잘못 캐스팅한 enum과 같이 배열의 범위를 벗어난 값이 들어오면, nullptr로 처리한다.
template <typename States, typename Id> auto* stateOf(States& states, Id id) noexcept {
    const auto index = static_cast<std::size_t>(id);
    return index < states.size() ? &states[index] : nullptr;
}

template <typename T> void addOnce(std::vector<T>& values, T value) {
    if (std::ranges::find(values, value) == values.end()) {
        values.push_back(value);
    }
}

// 제어 문자나 서로게이트 등 UTF-32 범위를 벗어난 문자는 무시한다. (빈 문자열 처리)
std::string toUtf8(char32_t c) {
    const bool control = c < 0x20 || (c >= 0x7F && c < 0xA0);
    const bool surrogate = c >= 0xD800 && c <= 0xDFFF;
    if (control || surrogate || c > 0x10FFFF) {
        return {};
    }

    std::string out;
    if (c < 0x80) {
        out += static_cast<char>(c);
    } else if (c < 0x800) {
        out += static_cast<char>(0xC0 | (c >> 6));
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        out += static_cast<char>(0xE0 | (c >> 12));
        out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (c >> 18));
        out += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    }
    return out;
}

} // namespace

// --- 틱 ----------------------------------------------------

Input::Tick Input::now() const noexcept {
    // update 밖에서 들어온 이벤트는 다음에 동작하는 update에서 처리한다.
    return Tick{frame_, TimeAccess::stepNumber() + 1};
}

bool Input::isCurrent(const Tick& tick) const noexcept {
    return TimeAccess::isInStep() ? tick.step == TimeAccess::stepNumber() : tick.frame == frame_;
}

template <typename T> T Input::current(const Accumulator<T>& accumulator) const {
    if (!isCurrent(accumulator.tick)) {
        return T{};
    }
    return TimeAccess::isInStep() ? accumulator.stepValue : accumulator.frameValue;
}

bool Input::press(ButtonState& state, const Tick& now) noexcept {
    if (state.down) {
        return false;
    }
    state.down = true;
    state.pressed = now;
    return true;
}

bool Input::release(ButtonState& state, const Tick& now) noexcept {
    if (!state.down) {
        return false;
    }
    state.down = false;
    state.released = now;
    return true;
}

void Input::applyButton(ButtonState* state, ButtonChange change) noexcept {
    if (state == nullptr) {
        return;
    }
    const Tick tick = now();
    if (change(*state, tick)) {
        refreshActions(tick);
    }
}

// --- 질의 ----------------------------------------------------

bool Input::isDown(Key key) const noexcept {
    const ButtonState* state = stateOf(keys_, key);
    return state != nullptr && state->down;
}

bool Input::wasPressed(Key key) const noexcept {
    const ButtonState* state = stateOf(keys_, key);
    return state != nullptr && isCurrent(state->pressed);
}

bool Input::wasReleased(Key key) const noexcept {
    const ButtonState* state = stateOf(keys_, key);
    return state != nullptr && isCurrent(state->released);
}

bool Input::isDown(MouseButton button) const noexcept {
    const ButtonState* state = stateOf(mouseButtons_, button);
    return state != nullptr && state->down;
}

bool Input::wasPressed(MouseButton button) const noexcept {
    const ButtonState* state = stateOf(mouseButtons_, button);
    return state != nullptr && isCurrent(state->pressed);
}

bool Input::wasReleased(MouseButton button) const noexcept {
    const ButtonState* state = stateOf(mouseButtons_, button);
    return state != nullptr && isCurrent(state->released);
}

Vec2i Input::mousePosition() const noexcept {
    return mousePosition_;
}

float Input::mouseWheelDelta() const noexcept {
    return current(wheel_);
}

std::string Input::text() const{
    return current(text_);
}

// --- 액션 --------------------------------------------------------------------

void Input::bind(std::string_view action, Key key) {
    Action& entry = actions_.try_emplace(std::string(action)).first->second;
    addOnce(entry.keys, key);
    entry.down = anyBoundDown(entry);
}

void Input::bind(std::string_view action, MouseButton button) {
    Action& entry = actions_.try_emplace(std::string(action)).first->second;
    addOnce(entry.buttons, button);
    entry.down = anyBoundDown(entry);
}

void Input::unbind(std::string_view action) {
    if (const auto it = actions_.find(action); it != actions_.end()) {
        actions_.erase(it);
    }
}

bool Input::isActionDown(std::string_view action) const {
    const Action* entry = findAction(action);
    return entry != nullptr && entry->down;
}

bool Input::wasActionPressed(std::string_view action) const {
    const Action* entry = findAction(action);
    return entry != nullptr && isCurrent(entry->pressed);
}

bool Input::wasActionReleased(std::string_view action) const {
    const Action* entry = findAction(action);
    return entry != nullptr && isCurrent(entry->released);
}

const Input::Action* Input::findAction(std::string_view name) const {
    const auto it = actions_.find(name);
    return it == actions_.end() ? nullptr : &it->second;
}

bool Input::anyBoundDown(const Action& action) const noexcept {
    return std::ranges::any_of(action.keys, [this](Key key) { return isDown(key); }) ||
           std::ranges::any_of(action.buttons,
                               [this](MouseButton button) { return isDown(button); });
}

void Input::refreshActions(const Tick& now) noexcept {
    for (Action& action : actions_ | std::views::values) {
        const bool down = anyBoundDown(action);
        if (down && !action.down) {
            action.pressed = now;
        } else if (!down && action.down) {
            action.released = now;
        }
        action.down = down;
    }
}

// --- 이벤트 ----------------------------------------------------

void Input::beginFrame() noexcept { ++frame_; }

void Input::handleEvent(const Event& event) {
    std::visit([this](const auto& e) { on(e); }, event);
}

void Input::on(const event::FocusLost&) {
    // 포커스가 없는 동안에는 뗀 키에 대해서 이벤트를 처리하지 않는다.
    // 눌린 채로 남아서 다음 update에 영향을 미치지 않도록 모두 떼어둔다.
     const Tick tick = now();
     for (ButtonState& state : keys_) {
        release(state, tick);
     }
     for (ButtonState& state: mouseButtons_) {
        release(state, tick);
     }
     refreshActions(tick);
}

void Input::on(const event::KeyPressed& event) {
    // Unknown에는 서로 다른 여러 키가 모이기 때문에 따로 추적하지 않는다.
    if (event.key != Key::Unknown) {
        applyButton(stateOf(keys_, event.key), press);
    }
}

void Input::on(const event::KeyReleased& event) {
    applyButton(stateOf(keys_, event.key), release);
}

void Input::on(const event::MouseButtonPressed& event) {
    mousePosition_ = event.position;
    applyButton(stateOf(mouseButtons_, event.button), press);
}

void Input::on(const event::MouseButtonReleased& event) {
    mousePosition_ = event.position;
    applyButton(stateOf(mouseButtons_, event.button), release);
}

void Input::on(const event::MouseMoved& event) {
    mousePosition_ = event.position;
}

void Input::on(const event::MouseWheelScrolled& event) {
    mousePosition_ = event.position;
    wheel_.add(event.delta, now());
}

void Input::on(const event::TextEntered& event) {
    const std::string utf8 = toUtf8(event.unicode);
    if (!utf8.empty()) {
        text_.add(utf8, now());
    }
}

}   // namespace meno
