# ---------------------------------------------------------------------------
# 예제 실행 파일
# ---------------------------------------------------------------------------
# 예제를 하나 추가할 때는 아래 함수를 한 줄 호출하면 된다.
# 각자 자기 예제만 추가하므로 이 파일에서의 충돌은 한 줄짜리다.
function(meno_add_example name)
    add_executable(meno_example_${name} ${name}/main.cpp)
    target_link_libraries(meno_example_${name} PRIVATE meno::meno)

    if(MSVC)
        target_compile_options(meno_example_${name} PRIVATE /utf-8)
    endif()
endfunction()

# 2D 렌더 API 전반을 한 화면에서 보여주는 예제. 에셋 파일 없이 동작한다.
meno_add_example(render_basics)

# Scene이 소유하는 GameObject와 기본 Sprite로 정적인 장면을 구성한다.
meno_add_example(gameobject_demo)
