#ifndef MENO_TEST_DISPLAY_HPP
#define MENO_TEST_DISPLAY_HPP

// 창이 필요한 테스트의 공통 규칙.
// 디스플레이가 없는 곳에서 창을 만들면 SFML이 프로세스를 abort한다.
// 창이 필요한 류의 테스트는 MENO_DISPLAY_TESTS=1일 때만 실제로 동작하고,
// 아니면 kSkipReturnCode로 끝맺은 뒤, ctest에 "건너뜀"으로 보고한다.

// 종료 코드는 tests/CMakeLists.txt가 정해 모든 테스트에 컴파일 정의로 넘긴다.
// ctest의 SKIP_RETURN_CODE(meno_display_test)와 같은 값을 한 곳에서 정하기 위해서다.
#ifndef MENO_SKIP_RETURN_CODE
#error "MENO_SKIP_RETURN_CODE가 없다. 테스트는 tests/CMakeLists.txt의 meno_add_test로 빌드할 것"
#endif

#include <cstdio>
#include <cstdlib>
#include <string_view>

namespace meno::test {

inline constexpr int kSkipReturnCode = MENO_SKIP_RETURN_CODE;

// 환경 변수. 없으면 빈 문자열.
[[nodiscard]] inline std::string_view env(const char* name) {
#ifdef _MSC_VER
#pragma warning(suppress : 4996) // getenv는 안전하지 않다고 경고한다. 이건 테스트용이므로 무시.
#endif
    const char* value = std::getenv(name);
    return value != nullptr ? value : "";
}

// 창이 필요한 테스트를 건너뛸 때 이유를 출력하고 true를 돌려준다. main의 첫 줄에서 쓴다.
//     if (meno::test::skipWithoutDisplay()) {
//         return meno::test::kSkipReturnCode;
//     }
[[nodiscard]] inline bool skipWithoutDisplay() {
    const std::string_view value = env("MENO_DISPLAY_TESTS");
    if (!value.empty() && value != "0") {
        return false;
    }
    std::printf("SKIP 창이 필요한 테스트. MENO_DISPLAY_TESTS=1일 때만 돈다.\n");
    return true;
}

} // namespace meno::test

#endif // MENO_TEST_DISPLAY_HPP
