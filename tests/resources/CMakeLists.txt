# SFML 대신 Texture/Font 테스트 대역을 사용해 장치 없이 캐시 동작을 검사한다.
meno_add_test(resource_manager
        ${CMAKE_SOURCE_DIR}/src/meno/resources/ResourceManager.cpp
)
