# Window/Renderer까지 소유하는 Game의 통합 루프와 종료 순서를 검사한다.
meno_add_test(game)
target_link_libraries(meno_test_game PRIVATE meno::meno)
meno_display_test(game)
