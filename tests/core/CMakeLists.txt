# 고정 timestep 루프와 시간 계층.
# 필요한 구현만 직접 컴파일해 SFML 없이 작동한다.
meno_add_test(application
        ${PROJECT_SOURCE_DIR}/src/meno/core/Application.cpp
        ${PROJECT_SOURCE_DIR}/src/meno/core/Clock.cpp
        ${PROJECT_SOURCE_DIR}/src/meno/core/Time.cpp
)

# Window/Renderer까지 소유하는 Game의 통합 루프와 종료 순서를 검사한다.
# 창이 필요하므로 일반 CI에서는 종료 코드 77로 건너뛴다.
meno_add_test(game)
target_link_libraries(meno_test_game PRIVATE meno::meno)
set_tests_properties(game PROPERTIES SKIP_RETURN_CODE 77)

meno_add_test(scene_manager
        ${PROJECT_SOURCE_DIR}/src/meno/core/SceneManager.cpp
        ${PROJECT_SOURCE_DIR}/src/meno/scene/Scene.cpp
)
