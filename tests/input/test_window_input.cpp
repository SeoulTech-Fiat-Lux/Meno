#include <meno/core/Window.hpp>
#include <meno/input/Input.hpp>

#include "check.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace {

constexpr int kSkipReturnCode = 77;

bool displayTestsEnabled() {
    const char* value = std::getenv("MENO_DISPLAY_TESTS");
    return value != nullptr && *value != '\0' && std::string_view{value} != "0";
}

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
    if (!displayTestsEnabled()) {
        std::printf("SKIP 창이 필요한 테스트. MENO_DISPLAY_TESTS = 1일 때만 돈다.\n");
        return kSkipReturnCode;
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