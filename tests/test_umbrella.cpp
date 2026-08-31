// 통합 헤더인 Meno.hpp가 실제로 컴파일되는지 확인한다.
//
// examples/ · tests/ · src/ 어디에서도 <meno/Meno.hpp>를 include하지 않는다.
// 그러므로 통합 헤더에 include 목록에 오류가 있어도 현재 파일로는 이를 바로잡을 수 없다.
// tools/check_header.py는 정규식으로 SFML의 유출만 확인할 뿐 컴파일을 하지는 않는다.
//
// 공개 헤더에 SFML이 등장하지 않는다는 것이 이 프로젝트의 규칙이므로, 이 타킷을 SFML을 링크하지 않고도 빌드된다.
// 만일 링크 오류가 발생한다면, 이는 현재 해당 규칙이 깨졌다는 것을 방증한다.

#include <meno/Meno.hpp>

int main() {
    return 0;
}