# ---------------------------------------------------------------------------
# meno 라이브러리 본체
# ---------------------------------------------------------------------------
# file(GLOB) 대신 소스가 눈에 보이도록 나열한다.
# GLOB은 파일이 추가돼도 cmake를 다시 돌리기 전까지 빌드에 반영되지 않아,
# 팀원이 git pull 한 후, Linkage에서 오류를 맞닥뜨리게 만든다.
# CONFIGURE_DEPENDS가 완화해 주지만 생성기마다 동작이 다르므로,
# 다소 하드코딩으로 보이더라도 소스의 이름을 명해두었다.
#
# 담당 경계: 새 소스를 추가할 때 자기 영역 아래에만 넣으면 충돌이 없다.
#   meno/core/       -- Core 담당 (Application, Window, Clock)
#   meno/math/       -- 공용 값 타입 (헤더 전용이라 여기 없음)
#   meno/graphics/   -- 2D 렌더러 담당
#   meno/sfml_backend/ -- SFML 접촉면. 각자 자기 *Access 구조체를 추가한다.
add_library(meno STATIC
        meno/core/Window.cpp
        meno/core/Collider.cpp
        meno/core/GameObject.cpp
        meno/graphics/Font.cpp
        meno/graphics/Renderer.cpp
        meno/graphics/Texture.cpp
)
add_library(meno::meno ALIAS meno)

target_include_directories(meno
        PUBLIC
        # 공개 헤더. 이 라이브러리를 쓰는 타깃에 전파된다.
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        # sfml_backend/*.hpp 를 "sfml_backend/..." 로 include 하기 위한 경로.
        # PRIVATE이라 사용자 타깃에는 전파되지 않는다 = 백엔드는 계속 감춰진다.
        # sfml과 상호작용하는 back_end 부분 구현 후 반영.
        ${CMAKE_CURRENT_SOURCE_DIR}/meno
)

# SFML은 PRIVATE으로 링크한다. 이게 "백엔드 감추기"의 빌드 시스템 쪽 절반이다.
# PUBLIC이면 meno를 쓰는 타깃에 SFML 헤더 경로가 전파되어,
# 사용자가 실수로 <SFML/...>을 include할 수 있게 된다 -> pimpl·friend 실현한 encapsulation이 무용지물이 됨.
#
# SFML::Graphics는 SFML::Window와 SFML::System을 전이 의존으로 끌고 온다.
# 오디오는 렌더 라이브러리의 의존이 아니므로 여기서 링크하지 않는다.
# (Audio 담당자가 자기 소스를 추가할 때 SFML::Audio를 붙이면 된다.)
target_link_libraries(meno PRIVATE SFML::Graphics)

target_compile_features(meno PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(meno PRIVATE /W4 /permissive-)
    # 소스와 실행 문자셋을 UTF-8로 고정한다. 한글 주석·문자열 리터럴이 깨지거나
    # C4819 경고가 쏟아지는 것을 막는다.
    target_compile_options(meno PRIVATE /utf-8)
else()
    target_compile_options(meno PRIVATE -Wall -Wextra -Wpedantic)
endif()
