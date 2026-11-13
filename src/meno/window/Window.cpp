#include <meno/input/Input.hpp>
#include <meno/window/Window.hpp>

#include "sfml_backend/Access.hpp"
#include "sfml_backend/InputConvert.hpp"
#include "sfml_backend/MathConvert.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <optional>

namespace meno {

struct Window::Impl {
    sf::RenderWindow window;
    Input input;

    Impl(Vec2u size, std::string_view title)
        : window(sf::VideoMode{backend::toSf(size)},
                 sf::String::fromUtf8(title.begin(), title.end())) {}
};

Window::Window(Vec2u size, std::string_view title) : impl_(std::make_unique<Impl>(size, title)) {}

Window::~Window() = default;

bool Window::isOpen() const { return impl_->window.isOpen(); }

void Window::close() { impl_->window.close(); }

void Window::pollEvents() {
    impl_->input.beginFrame();
    while (const std::optional event = impl_->window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            impl_->window.close();
        } else if (const std::optional converted = backend::fromSf(*event)) {
            impl_->input.handleEvent(*converted);
        }
    }
}

Input& Window::input() { return impl_->input; }

const Input& Window::input() const { return impl_->input; }

Vec2u Window::size() const { return backend::fromSf(impl_->window.getSize()); }

namespace backend {

sf::RenderWindow& WindowAccess::native(const Window& window) noexcept {
    // unique_ptr의 const는 얕다. const Window&여도 *impl_는 non-const Impl&이므로
    // const_cast 없이 그릴 수 있는 RenderWindow를 꺼낼 수 있다.
    return window.impl_->window;
}

} // namespace backend

} // namespace meno
