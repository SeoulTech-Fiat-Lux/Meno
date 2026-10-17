# 렌더 계층 테스트.
# meno 라이브러리(=SFML)를 링크하지만 공개헤더만 include한다.
# 런타임 검사는 창과 GL 컨텍스트가 필요하다.
# 디스플레이가 없으면 SFML이 프로세스를 abort하므로 MENO_DISPLAY_TESTS=1일 때만 돌고,
# 아니면 종료 코드 77을 돌려 ctest에 "건너뜀"으로 보고한다. 컴파일 타임 검사(static_assert)는 빌드할 때 항상 돈다.
# CI의 Linux 잡은 xvfb-run으로 가상 디스플레이를 띄워서 돌린다.
meno_add_test(render)
target_link_libraries(meno_test_render PRIVATE meno::meno)
set_tests_properties(render PROPERTIES SKIP_RETURN_CODE 77)
