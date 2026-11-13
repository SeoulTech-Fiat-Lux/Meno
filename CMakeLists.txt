# SFML 3.1.0의 최상단 CMakeLists.txt가 cmake_minimum_required(VERSION 3.28)을 선언하므로, 수정.
cmake_minimum_required(VERSION 3.28)

project(meno
        VERSION 0.1.0
        DESCRIPTION "SFML 기반 2D 게임 개발 라이브러리"
        LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# 전역 설정
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 아래 기본값은 meno를 직접 빌드할 때만 적용.
# 다른 프로젝트가 add_subdirectory나 FetchContent로 가져가면 해당 빌드 타입은 건드리지 않는다.
if(PROJECT_IS_TOP_LEVEL)
    # clangd / VS Code IntelliSense가 참조하는 compile_commands.json 생성
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    endif()
endif()

option(MENO_BUILD_EXAMPLES "예제 실행 파일을 빌드한다." ${PROJECT_IS_TOP_LEVEL})
option(MENO_BUILD_TESTS "테스트를 빌드한다." ${PROJECT_IS_TOP_LEVEL})

# ---------------------------------------------------------------------------
# 의존성: SFML
# ---------------------------------------------------------------------------
include(FetchContent)

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC      OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK  OFF CACHE BOOL "" FORCE)

# SFML 3.1 기준, 캐시로 쓰면 meno를 가져다 쓰는 프로젝트의 설정까지 바뀌므로 일반 변수 활용.
set(BUILD_SHARED_LIBS   OFF)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        3.1.0
        GIT_SHALLOW    ON
        # SYSTEM: SFML 헤더를 시스템 헤더로 취급해 -Wall -Wextra 경고가
        #         우리 빌드 로그를 덮지 않게 한다.
        SYSTEM
        # EXCLUDE_FROM_ALL: SFML의 install 타깃이 우리 install에 섞이지 않게 한다.
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SFML)

# ---------------------------------------------------------------------------
# 하위 디렉터리
# ---------------------------------------------------------------------------
# 디렉터리마다 CMakeLists.txt를 따로 두는 이유:
# 3명이 각자 소스를 추가할 때 이 파일 하나를 같이 고치면 머지 충돌이 반드시 난다.
# 각자 자기 담당 디렉터리의 파일만 편집하면 충돌이 거의 사라진다.
#
#   src/CMakeLists.txt       -- 라이브러리 본체
#   examples/CMakeLists.txt  -- 데모 실행 파일들
#   tests/CMakeLists.txt     -- 테스트
add_subdirectory(src)

if(MENO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(MENO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# 이전 GameObjectDemo 타깃에 대해
# ---------------------------------------------------------------------------
# src/gameobject_demo/ 와 src/main.cpp 는 그대로 두었다(상준 담당).
# 다만 이 브랜치에서는 빌드 대상에 넣지 않는다.
# GameObject.h 가 포함하는 Vec2D.h 가 이 브랜치에 없어서 컴파일되지 않기 때문.
# GameObject-Demo 브랜치를 병합할 때 examples/gameobject_demo/로 옮기고 examples/CMakeLists.txt에 타깃을 추가하거나, 다른 방법을 쓰면 되겠다.