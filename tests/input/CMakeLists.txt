# Input의 상태 판정. Application 루프와 함께 돌려 update 기준 판정까지 SFML 없이 검증한다.
meno_add_test(input
        ${PROJECT_SOURCE_DIR}/src/meno/input/Input.cpp
        ${PROJECT_SOURCE_DIR}/src/meno/core/Application.cpp
        ${PROJECT_SOURCE_DIR}/src/meno/core/Clock.cpp
        ${PROJECT_SOURCE_DIR}/src/meno/core/Time.cpp
)

# Input.cpp가 내부 헤더 core/TimeAccess.hpp를 쓰므로 src/meno를 include 경로에 넣는다.
target_include_directories(meno_test_input PRIVATE ${PROJECT_SOURCE_DIR}/src/meno)

# SFML 이벤트 -> meno 이벤트 변환. 내부 헤더(sfml_backend/)를 직접 검사하므로
# src/meno를 include 경로에 넣고 SFML을 함께 링크한다. 창은 열지 않는다.
meno_add_test(input_convert)
target_include_directories(meno_test_input_convert PRIVATE ${PROJECT_SOURCE_DIR}/src/meno)
target_link_libraries(meno_test_input_convert PRIVATE meno::meno SFML::Window)

# Window가 Input을 갖고 이벤트를 넘기는 연결부. 창을 열어야 하므로 render 테스트처럼
# MENO_DISPLAY_TESTS=1일 때만 돈다.
meno_add_test(window_input)
target_link_libraries(meno_test_window_input PRIVATE meno::meno)
meno_display_test(window_input)
