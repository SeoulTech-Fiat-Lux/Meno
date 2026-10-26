# 고정 timestep 루프와 시간 계층.
# 필요한 구현만 직접 컴파일해 SFML 없이 작동한다.
meno_add_test(application
        ${CMAKE_SOURCE_DIR}/src/meno/core/Application.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/core/Clock.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/core/Time.cpp
)

meno_add_test(scene_manager
        ${CMAKE_SOURCE_DIR}/src/meno/core/SceneManager.cpp
)
