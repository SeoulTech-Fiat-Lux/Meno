#ifndef MENO_INPUT_CONVERT_HPP
#define MENO_INPUT_CONVERT_HPP

#include <meno/input/Event.hpp>
#include <meno/input/Key.hpp>
#include <meno/input/MouseButton.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <optional>

// SFML 입력 타입을 meno 입력 타입으로 변환하는 내부 전용 헤더.

// MathConvert.h와 역할이 같으나, SFML/Window 헤더를 끌고 오므로 따로 두었다.
// 렌더러와 같이, 입력과 관련 없는 구현부가 위 헤더를 포함하지 않게 하기 위해서 취한 조치이다.
namespace meno::backend {

// 대응하는 키가 없으면 Key::Unknown
[[nodiscard]] Key fromSf(sf::Keyboard::Key key) noexcept;

[[nodiscard]] std::optional<MouseButton> fromSf(sf::Mouse::Button button) noexcept;

// Input이 다루는 이벤트만 변환한다.
// 닫기나 크기 변경, 가로 휠과 같이 Input과 무관한 이벤트는 std::nullopt로 처리한다.
[[nodiscard]] std::optional<Event> fromSf(const sf::Event& event) noexcept;

} // namespace meno::backend


#endif // MENO_INPUT_CONVERT_HPP