#include <meno/core/Window.hpp>

#include "sfml_backend/Access.hpp"
#include "sfml_backend/Convert.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <utility>

namespace meno {

struct Window::Impl {
    sf::RenderWindow window;

    Impl(Vec2u size, std::string_view title)
        // SFML 3의 VideoMode는 폭/높이를 따로 받지 않고 Vector2u 하나를 받는다.
        : window(sf::VideoMode{backend::toSf(size)},
                 sf::String::fromUtf8(title.begin(), title.end())) {}
};

Window::Window(Vec2u size, std::string_view title)
    : impl_(std::make_unique<Impl>(size, title)) {}

Window::~Window() = default;

Window::Window(Window&&) noexcept = default;

Window& Window::operator=(Window&&) noexcept = default;

bool Window::isOpen() const {
    return impl_ != nullptr && impl_->window.isOpen();
}

void Window::close() {
    impl_->window.close();
}

void Window::pollEvents() {
    // SFML 3의 pollEvent는 std::optional<sf::Event>를 돌려준다.
    while (const std::optional event = impl_->window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            impl_->window.close();
        }
    }
}

Vec2u Window::size() const {
    return backend::fromSf(impl_->window.getSize());
}

namespace backend {

sf::RenderWindow* WindowAccess::native(const Window& window) noexcept {
    // unique_ptr의 const는 얕다. const Window&여도 *impl_는 non-const Impl&이므로
    // const_cast 없이 그릴 수 있는 RenderWindow를 꺼낼 수 있다.
    if (window.impl_ == nullptr) {
        return nullptr;
    }
    return &window.impl_->window;
}

} // namespace backend

} // namespace meno
