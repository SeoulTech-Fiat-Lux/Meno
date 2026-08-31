# ---------------------------------------------------------------------------
# 테스트
# ---------------------------------------------------------------------------
# 지금은 의존성 없는 자체 검사(assert + 종료 코드)로 되어 있다.
# 테스트 프레임워크(Catch2)가 확정 이후, FetchContent로 받아 이 파일만 고치면 된다.
#
# 값 타입 테스트는 SFML을 링크하지 않는다.
# 헤더만으로 성립하므로 SFML 빌드(첫 회 약 10분) 없이도 점검할 수 있다.
#
# # meno_add_test(<name> [<sources>...])
#
# # test_<name>.cpp를 컴파일한 뒤, meno_test_<name> 실행 파일과 같은 이름의 ctest 항목을 추가한다.
# 만일 구현 소스가 필요한 테스트의 경우, 뒤에 경로를 덧붙인다.
function(meno_add_test name)
    add_executable(meno_test_${name} test_${name}.cpp ${ARGN})
    target_include_directories(meno_test_${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_features(meno_test_${name} PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(meno_test_${name} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(meno_test_${name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${name} COMMAND meno_test_${name})
endfunction()

meno_add_test(math)

# 통합 헤더가 컴파일되는지만 확인한다. 검사할 동작이 없으므로 해당 소스 코트는 한 줄뿐이다.
meno_add_test(umbrella)

meno_add_test(application
        ${CMAKE_SOURCE_DIR}/src/meno/core/Application.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/core/Clock.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/core/Time.cpp
)