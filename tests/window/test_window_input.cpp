#include <meno/window/Window.hpp>
#include <meno/input/Input.hpp>

#include "check.hpp"
#include "display.hpp"

#include <stdexcept>
#include <utility>

namespace {

template <typename F> bool throwsLogicError(F&& call) {
    try {
        call();
    } catch (const std::logic_error&) {
        return true;
    }
    return false;
}

} // namespace

int main() {
    if (meno::test::skipWithoutDisplay()) {
        return meno::test::kSkipReturnCode;
    }

    meno::Window window{{160u, 90u}, "meno input test"};
    meno::Input& input = window.input();
    input.bind("jump", meno::Key::Space);

    // 이벤트가 없어도 pollEvents는 입력 상태를 건드리지 않고 프레임만 넘긴다.
    window.pollEvents();
    MENO_CHECK(!input.isDown(meno::Key::Space));
    MENO_CHECK(!input.isActionDown("jump"));

    // Input은 Impl 안에 있으므로 이동 생성한 Window가 같은 Input을 그대로 갖는다.
    meno::Window moved{std::move(window)};
    MENO_CHECK(&moved.input() == &input);
    moved.input().handleEvent(meno::event::KeyPressed{meno::Key::Space});
    MENO_CHECK(input.isActionDown("jump"));

    // 이동당한 Window에는 Input이 없다.
    MENO_CHECK(throwsLogicError([&] { (void)window.input(); }));
    const meno::Window& movedFrom = window;
    MENO_CHECK(throwsLogicError([&] { (void)movedFrom.input(); }));
    window.pollEvents(); // 이동당한 Window의 pollEvents는 아무것도 하지 않는다.

    return meno::test::report();
}