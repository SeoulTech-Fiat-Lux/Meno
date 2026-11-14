# Sound, Music. 볼륨 변환(src/meno/audio/Volume.hpp)도 검사하므로 내부 헤더 경로를 추가한다.
meno_add_test(audio)

target_include_directories(meno_test_audio PRIVATE ${PROJECT_SOURCE_DIR}/src/meno)

target_link_libraries(meno_test_audio PRIVATE meno::meno)
