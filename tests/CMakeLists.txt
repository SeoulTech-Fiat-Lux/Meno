# 지금은 의존성 없는 자체 검사(check.hpp + 종료 코드)로 되어 있다.
# 테스트 프레임워크(Catch2)가 확정 이후, FetchContent로 받아 이 파일만 고치면 된다.

# 담당 경계: 테스트는 모듈별 하위 디렉터리에 두고, 그 디렉터리의 CMakeLists.txt에서 등록한다.
# 이 파일은 공용 함수와 디렉터리 목록만 가지므로, 테스트를 추가할 때는 고치지 않는다.
# (최상위 CMakeLists.txt가 src/ examples/ tests/를 나눈 것과 같은 이유다.)

#   tests/math/      -- 공용 값 타입
#   tests/core/      -- Application, Clock, Time
#   tests/graphics/  -- Renderer, Texture, Font
#   tests/audio/     -- Sound, Music
#   tests/resources/ -- Texture, Font 캐시
#   tests/scene/     -- Scene, GameObject, Component, Collider

# 테스트를 추가할 때는 대상 코드가 무엇에 의존하는지에 따라 아래 셋 중 하나를 고른다.
# 가능한 한 위쪽을 고른다. SFML 빌드 없이 돌고, 실패했을 때 원인도 좁아진다.

#   1. 헤더만 쓰는 코드       meno_add_test(name)                          예: math
#   2. SFML 없는 구현         meno_add_test(name <구현 .cpp들>)            예: application
#      *구현 일부를 테스트 대역으로 바꿀 때도 이 방식을 쓰고, meno는 링크하지 않는다.  예: audio_component
#   3. SFML이 필요한 구현     meno_add_test(name)
#                             target_link_libraries(meno_test_<name> PRIVATE meno::meno)  예: render, scene

# 값 타입 테스트는 SFML을 링크하지 않는다.
# 헤더만으로 성립하므로 SFML 빌드(첫 회 약 10분) 없이도 점검할 수 있다.

# # meno_add_test(<name> [<sources>...])

# # 호출한 디렉터리의 test_<name>.cpp를 컴파일한 뒤, meno_test_<name> 실행 파일과 같은 이름의 ctest 항목을 추가한다.
# 만일 구현 소스가 필요한 테스트의 경우, 뒤에 경로를 덧붙인다.
# 테스트 이름은 디렉터리가 달라도 겹치면 안 된다(타깃 이름이 전역이다).
function(meno_add_test name)
    add_executable(meno_test_${name} test_${name}.cpp ${ARGN})
        target_include_directories(meno_test_${name} PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/tests   # check.hpp
    )
    target_compile_features(meno_test_${name} PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(meno_test_${name} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(meno_test_${name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${name} COMMAND meno_test_${name})

    # 아래 등록 누락 검사에서 쓴다.
    set_property(GLOBAL APPEND PROPERTY MENO_REGISTERED_TESTS
            ${CMAKE_CURRENT_SOURCE_DIR}/test_${name}.cpp)
endfunction()

# 통합 헤더가 컴파일되는지만 확인한다. 검사할 동작이 없으므로 해당 소스 코드는 한 줄뿐이다.
meno_add_test(umbrella)

# 알파벳 순으로 한 줄에 하나씩 둔다. 끝에 덧붙이지 않아야 두 브랜치가 동시에 모듈을 추가해도
# 서로 다른 줄에 끼워 넣게 되어 충돌이 잘 나지 않는다.
add_subdirectory(audio)
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(math)
add_subdirectory(resources)
add_subdirectory(scene)

# ---------------------------------------------------------------------------
# 등록 누락 검사
# ---------------------------------------------------------------------------
# test_*.cpp를 만들고 meno_add_test 등록을 빠뜨리면 CI에서 조용히 빠진다.
# 머지 충돌을 해결하다 등록 줄이 사라져도 마찬가지다(PR #19에서 실제로 있었던 일).
# 그래서 configure 단계에서 파일 목록과 등록 목록을 비교해 바로 실패시킨다.
file(GLOB_RECURSE meno_test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)
get_property(meno_registered_tests GLOBAL PROPERTY MENO_REGISTERED_TESTS)
foreach(source IN LISTS meno_test_sources)
    if(NOT source IN_LIST meno_registered_tests)
        file(RELATIVE_PATH relative ${CMAKE_SOURCE_DIR} ${source})
        message(FATAL_ERROR
                "${relative}가 등록되지 않았다. "
                "같은 디렉터리의 CMakeLists.txt에 meno_add_test를 추가할 것.")
    endif()
endforeach()
