# ---------------------------------------------------------------------------
# 예제 실행 파일
# ---------------------------------------------------------------------------
# 예제를 추가할 때는 아래 함수를 한 줄 호출하고, README.md의 표에도 한 줄을 더한다. 예제 설명과
# 조작 방법은 main.cpp 맨 위의 주석에 쓴다. 각자 자기 예제의 줄만 추가하므로, 이 파일에서 충돌이
# 나도 한 줄에 그친다.
function(meno_add_example name)
    add_executable(meno_example_${name} ${name}/main.cpp)
    target_link_libraries(meno_example_${name} PRIVATE meno::meno)

    if(MSVC)
        target_compile_options(meno_example_${name} PRIVATE /utf-8)
    endif()
endfunction()

# 2D 렌더 API의 주요 기능을 한 화면에서 보여 준다. Game 없이 루프를 직접 구현한다.
meno_add_example(render_basics)

# Scene이 소유하는 GameObject와 기본 Sprite로 정적인 장면을 구성한다.
meno_add_example(gameobject_demo)

# 키보드·마우스 입력과 동작 이름 묶기. Application의 update()에서 입력을 읽는다.
meno_add_example(input_demo)

# 두 Scene을 3초마다 자동으로 전환하며 Scene 핸들러와 예약 전환을 보여 준다.
meno_add_example(scene_transition)

# 바닥과 상자 GameObject, 충돌 판정으로 저절로 오가며 점프하는 상자를 만든다.
meno_add_example(jumping_box)
