#include <meno/window/Window.hpp>
#include <meno/input/Input.hpp>

#include "check.hpp"
#include "display.hpp"

#include <type_traits>

// Window는 복사도 이동도 할 수 없다. Renderer가 붙잡은 렌더 타깃과 input()의 참조가 끝까지 유효한 근거다.
static_assert(!std::is_copy_constructible_v<meno::Window>);
static_assert(!std::is_copy_assignable_v<meno::Window>);
static_assert(!std::is_move_constructible_v<meno::Window>);
static_assert(!std::is_move_assignable_v<meno::Window>);

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

    // input()은 부를 때마다 같은 Input을 돌려준다. const 쪽도 같다.
    MENO_CHECK(&window.input() == &input);
    const meno::Window& constWindow = window;
    MENO_CHECK(&constWindow.input() == &input);

    // Window를 거쳐 넣은 이벤트가 미리 받아 둔 참조에도 보인다.
    window.input().handleEvent(meno::event::KeyPressed{meno::Key::Space});
    MENO_CHECK(input.isActionDown("jump"));

    return meno::test::report();
}