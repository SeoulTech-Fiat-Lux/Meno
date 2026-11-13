#ifndef MENO_TIME_ACCESS_HPP
#define MENO_TIME_ACCESS_HPP

#include <cstdint>

// Time의 update 단계 정보를 라이브러리 내부(Input)에서만 읽기 전용으로 연다.
// sfml_backend/Access.hpp와 같은 방식이지만, SFML 없이 도는 Input 테스트를 위해 따로 둔다.

namespace meno::detail {

struct TimeAccess {
    // 지금까지 수행된 fixed update 수. run을 다시 시작해도 초기화되지 않는다.
    [[nodiscard]] static std::uint64_t stepNumber() noexcept;
    // 현재 fixed update(Application::update) 안인지 점검하는 bool 함수.
    [[nodiscard]] static bool isInStep() noexcept;
};

} // namespace meno::detail

#endif // MENO_TIME_ACCESS_HPP
