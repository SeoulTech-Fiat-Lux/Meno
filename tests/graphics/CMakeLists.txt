# 카메라 수학과 SFML의 대조. backend::toSfView로 만든 뷰를 SFML의 좌표 변환에 넣어
# math/Camera2D.hpp의 결과와 비교한다. 크기만 가진 가짜 렌더 타깃을 쓰므로 창도 디스플레이도 필요 없다.
# 내부 헤더(sfml_backend/Convert.hpp)와 sf:: 타입을 직접 쓰므로 내부 헤더 경로와 SFML을 연결한다.
# meno 본체는 링크하지 않는다. 대조하는 코드가 전부 헤더에 있기 때문이다.
meno_add_test(camera_parity)
target_include_directories(meno_test_camera_parity PRIVATE ${CMAKE_SOURCE_DIR}/src/meno)
target_link_libraries(meno_test_camera_parity PRIVATE SFML::Graphics)

# 렌더 계층 테스트.
# meno 라이브러리(=SFML)를 링크하지만 공개헤더만 include한다.
# 런타임 검사는 창과 GL 컨텍스트가 필요하다.
# 디스플레이가 없으면 SFML이 프로세스를 abort하므로 MENO_DISPLAY_TESTS=1일 때만 돌고,
# 아니면 종료 코드 77을 돌려 ctest에 "건너뜀"으로 보고한다. 컴파일 타임 검사(static_assert)는 빌드할 때 항상 돈다.
# CI의 Linux 잡은 xvfb-run으로 가상 디스플레이를 띄워서 돌린다.
meno_add_test(render)
target_link_libraries(meno_test_render PRIVATE meno::meno)
set_tests_properties(render PROPERTIES SKIP_RETURN_CODE 77)

# 창 크기를 바꾼 뒤의 Renderer와 뷰포트가 실제로 그린 픽셀. render와 같이 디스플레이가 필요하다.
# 창 크기를 바꾸고 픽셀을 읽으려고 내부 헤더(sfml_backend/Access.hpp)로 sf::RenderWindow를 꺼내므로,
# 내부 헤더 경로와 SFML을 직접 연결한다. 그리기는 공개 API(Renderer)로만 한다.
meno_add_test(render_resize)
target_include_directories(meno_test_render_resize PRIVATE ${CMAKE_SOURCE_DIR}/src/meno)
target_link_libraries(meno_test_render_resize PRIVATE meno::meno SFML::Graphics)
set_tests_properties(render_resize PROPERTIES SKIP_RETURN_CODE 77)
