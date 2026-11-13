# meno를 하위 프로젝트로 포함하는 사용자 프로젝트의 최소 전형.
# 점검의 목적이 다르므로 tests/CMakeLists.txt는 이 디렉터리를 포함하지 않는다.
#
#     cmake -S tests/consumer -B build-consumer
#     cmake --build build-consumer --target game
#
# FetchContent_MakeAvailable(meno)도 안에서 add_subdirectory를 부르므로, 이 경로 하나로 둘 다 검사된다.
cmake_minimum_required(VERSION 3.28)
project(meno_consumer LANGUAGES CXX)

# 사용자 프로젝트가 정해 둔 값. meno를 추가한 뒤에도 그대로여야 한다.
set(BUILD_SHARED_LIBS ON CACHE BOOL "")
set(build_type_before "$CACHE{CMAKE_BUILD_TYPE}")

add_subdirectory(../.. meno)

if(NOT "$CACHE{BUILD_SHARED_LIBS}")
    message(FATAL_ERROR "meno가 사용자 프로젝트의 BUILD_SHARED_LIBS 캐시를 바꿨다.")
endif()
if(NOT "$CACHE{CMAKE_BUILD_TYPE}" STREQUAL "${build_type_before}")
    message(FATAL_ERROR "meno가 사용자 프로젝트의 CMAKE_BUILD_TYPE을 바꿨다: "
                        "'${build_type_before}' -> '$CACHE{CMAKE_BUILD_TYPE}'")
endif()
if(MENO_BUILD_TESTS OR MENO_BUILD_EXAMPLES)
    message(FATAL_ERROR "하위 프로젝트일 때는 meno의 테스트와 예제가 기본으로 꺼져 있어야 한다.")
endif()

add_executable(game main.cpp)
target_link_libraries(game PRIVATE meno::meno)