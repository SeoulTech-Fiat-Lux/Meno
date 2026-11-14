# 사용자 설명서, README, API 레퍼런스 첫 쪽(docs/mainpage.md)의 C++ 코드 블록이 컴파일되는지 확인한다.
# 코드는 Markdown 안에만 있으므로 configure 때 tools/extract_guide_code.py가 블록마다 .cpp를 만든다.
# 완성 프로그램은 meno와 링크까지 하고, 조각은 컴파일만 한다. 창을 열기 때문에 실행하지는 않는다.
# 컴파일 오류는 생성된 파일이 아니라 Markdown 파일의 줄을 가리킨다.

find_package(Python3 COMPONENTS Interpreter QUIET)
option(MENO_BUILD_GUIDE_CODE "설명서와 README의 C++ 코드 블록을 컴파일한다. Python 3가 필요하다."
        ${Python3_FOUND})
if(NOT MENO_BUILD_GUIDE_CODE)
    message(STATUS "설명서 코드 검사를 건너뛴다(MENO_BUILD_GUIDE_CODE=OFF).")
    return()
endif()
if(NOT Python3_FOUND)
    message(FATAL_ERROR "MENO_BUILD_GUIDE_CODE에는 Python 3가 필요하다. "
            "Python을 설치하거나 -DMENO_BUILD_GUIDE_CODE=OFF로 끌 것.")
endif()

file(GLOB meno_guide_docs CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/docs/guide/*.md")
list(PREPEND meno_guide_docs "${PROJECT_SOURCE_DIR}/README.md" "${PROJECT_SOURCE_DIR}/docs/mainpage.md")
set(meno_guide_extractor "${PROJECT_SOURCE_DIR}/tools/extract_guide_code.py")
set(meno_guide_blocks "${CMAKE_CURRENT_BINARY_DIR}/blocks")

execute_process(
        COMMAND "${Python3_EXECUTABLE}" "${meno_guide_extractor}"
                --root "${PROJECT_SOURCE_DIR}" --out "${meno_guide_blocks}" ${meno_guide_docs}
        RESULT_VARIABLE meno_guide_result
        OUTPUT_VARIABLE meno_guide_output
        ERROR_VARIABLE meno_guide_error
        OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT meno_guide_result EQUAL 0)
    message(FATAL_ERROR "설명서 코드를 뽑지 못했다.\n${meno_guide_error}")
endif()
message(STATUS "설명서 코드: ${meno_guide_output}")

# 문서나 스크립트를 고치면 다음 빌드가 configure부터 다시 해서 블록을 새로 뽑는다.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${meno_guide_docs} "${meno_guide_extractor}")

# 경고 수준은 테스트와 같다. 오류만 빌드를 실패시킨다.
function(meno_guide_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# 완성 프로그램: 블록마다 실행 파일 하나. 사용자가 복사해 빌드하는 것과 같게 meno와 링크한다.
file(GLOB meno_guide_programs "${meno_guide_blocks}/program_*.cpp")
foreach(source IN LISTS meno_guide_programs)
    get_filename_component(name "${source}" NAME_WE)
    string(REGEX REPLACE "^program_" "meno_guide_" target "${name}")
    add_executable(${target} "${source}")
    target_link_libraries(${target} PRIVATE meno::meno)
    meno_guide_warnings(${target})
endforeach()

# 조각: 링크하지 않으므로 meno 대신 공개 include 경로만 준다.
file(GLOB meno_guide_fragments "${meno_guide_blocks}/fragment_*.cpp")
if(meno_guide_fragments)
    add_library(meno_guide_fragments OBJECT ${meno_guide_fragments})
    target_include_directories(meno_guide_fragments PRIVATE "${PROJECT_SOURCE_DIR}/include")
    target_compile_features(meno_guide_fragments PRIVATE cxx_std_20)
    meno_guide_warnings(meno_guide_fragments)
    # 조각은 만든 값을 그 블록 안에서 쓰지 않는 경우가 많다.
    if(MSVC)
        target_compile_options(meno_guide_fragments PRIVATE /wd4189)
    else()
        target_compile_options(meno_guide_fragments PRIVATE
                -Wno-unused-variable -Wno-unused-but-set-variable)
    endif()
endif()
