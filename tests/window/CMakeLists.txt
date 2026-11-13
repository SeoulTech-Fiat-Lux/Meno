# Window가 Input을 갖고 이벤트를 넘기는 연결부.
meno_add_test(window_input)
target_link_libraries(meno_test_window_input PRIVATE meno::meno)
meno_display_test(window_input)
