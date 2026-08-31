#pragma once

// assert()는 사용하지 않는다.
// CI가 -DCMAKE_BUILD_TYPE=Release로 빌드할 때 NDEBUG가 정의되므로, assert()가 전처리 단계에서 전부 사라진다.
// 그러므로 테스트는 아무 것도 검사하지 않은 채로 0을 반환한다.
// 따라서 실패를 세어 두었다가 main의 반환값으로 넘겨야 ctest가 테스트의 실패를 인식할 수 있다.

#include <cstdio>

namespace meno::test {

inline int failureCount = 0;

// 설명 문구를 직접 붙이고 싶을 때 사용한다. 식을 그대로 남기려면 MENO_CHECK를 활용하자.
inline void check(bool ok, const char* what) {
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        ++failureCount;
    }
}

// main의 마지막에 호출하여 그 반환값을 그대로 종료 코드로 넘기는 용도이다. ctest가 실패를 인식하게 된다.
[[nodiscard]] inline int report() {
    std::printf("\n%s (%d failure%s)\n", failureCount == 0 ? "ALL PASS" : "FAILED",
                failureCount, failureCount == 1 ? "" : "s");
    return failureCount == 0 ? 0 : 1;
}

} // namespace meno::test

// 식 안에 템플릿 인자와 같은 최상위 쉼표가 있어도 쪼개지지 않도록 가변인자로 받는다.
#define MENO_CHECK(...) meno::test::check(static_cast<bool>(__VA_ARGS__), #__VA_ARGS__)