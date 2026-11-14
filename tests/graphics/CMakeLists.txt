# 카메라 수학과 SFML의 대조. backend::toSfView로 만든 뷰를 SFML의 좌표 변환에 넣어
# math/Camera2D.hpp의 결과와 비교한다. 크기만 가진 가짜 렌더 타깃을 쓰므로 창도 디스플레이도 필요 없다.
# 내부 헤더(sfml_backend/MathConvert.hpp)와 sf:: 타입을 직접 쓰므로 내부 헤더 경로와 SFML을 연결한다.
# meno 본체는 링크하지 않는다. 대조하는 코드가 전부 헤더에 있기 때문이다.
meno_add_test(camera_parity)
target_include_directories(meno_test_camera_parity PRIVATE ${PROJECT_SOURCE_DIR}/src/meno)
target_link_libraries(meno_test_camera_parity PRIVATE SFML::Graphics)

# 렌더 계층 테스트.
# meno 라이브러리(=SFML)를 링크하지만 공개 헤더만 include한다.
# 런타임 검사는 창과 GL 컨텍스트가 필요하다.
# 컴파일 타임 검사(static_assert)는 빌드할 때 항상 실행된다.
meno_add_test(render)
target_link_libraries(meno_test_render PRIVATE meno::meno)
meno_display_test(render)

# 창 크기를 바꾼 뒤의 Renderer와, 뷰포트가 실제로 그린 픽셀. render와 같이 디스플레이가 필요하다.
# 창 크기를 바꾸고 픽셀을 읽으려고 내부 헤더(sfml_backend/Access.hpp)로 sf::RenderWindow를 꺼내므로,
# 내부 헤더 경로와 SFML을 직접 연결한다. 그리기는 공개 API(Renderer)로만 한다.
meno_add_test(render_resize)
target_include_directories(meno_test_render_resize PRIVATE ${PROJECT_SOURCE_DIR}/src/meno)
target_link_libraries(meno_test_render_resize PRIVATE meno::meno SFML::Graphics)
meno_display_test(render_resize)

# 기본 도형(사각형·원·선·다각형·부채꼴)이 실제로 칠하는 픽셀. render와 같이 디스플레이가 필요하다.
# 픽셀을 읽으려고 render_resize처럼 내부 헤더(sfml_backend/Access.hpp)로 sf::RenderWindow를 꺼내므로,
# 내부 헤더 경로와 SFML을 직접 연결한다.
meno_add_test(render_shapes)
target_include_directories(meno_test_render_shapes PRIVATE ${PROJECT_SOURCE_DIR}/src/meno)
target_link_libraries(meno_test_render_shapes PRIVATE meno::meno SFML::Graphics)
meno_display_test(render_shapes)
