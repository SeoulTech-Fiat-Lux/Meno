# Input의 상태 판정.
# SFML 없이 가상의 Application 루프 위에서 update 기준으로 입력의 정상 동작 여부를 판정한다.
meno_add_test(input
        ${CMAKE_SOURCE_DIR}/src/meno/input/Input.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/core/Application.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/core/Clock.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/core/Time.cpp
)
