# Sound 테스트 대역으로 컴포넌트 동작과 GameObject 수명을 장치 없이 검증한다.
# 대역과 겹치지 않도록 meno를 링크하지 않고 필요한 구현만 직접 컴파일한다.
meno_add_test(audio_component
        ${CMAKE_SOURCE_DIR}/src/meno/scene/Audio.cpp
        ${CMAKE_SOURCE_DIR}/src/meno/scene/Scene.cpp
)

meno_add_test(collider)
target_link_libraries(meno_test_collider PRIVATE meno::meno)

meno_add_test(gameobject)
target_link_libraries(meno_test_gameobject PRIVATE meno::meno)

meno_add_test(scene)
target_link_libraries(meno_test_scene PRIVATE meno::meno)
