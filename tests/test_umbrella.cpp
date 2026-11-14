// 통합 헤더인 Meno.hpp가 실제로 컴파일되는지 확인한다.
//
// src/는 <meno/Meno.hpp>를 include하지 않으므로, 라이브러리만 빌드해서는 통합 헤더의 include
// 목록에 있는 오류를 잡을 수 없다.
// tools/check_headers.py는 정규식으로 SFML의 유출만 확인할 뿐 컴파일하지는 않는다.
//
// 공개 헤더에 SFML이 등장하지 않는다는 것이 이 프로젝트의 규칙이므로, 이 타깃은 SFML을 링크하지
// 않고도 빌드된다. SFML 때문에 빌드가 실패한다면 이 규칙이 깨진 것이다.

#include <meno/Meno.hpp> // IWYU pragma: keep

int main() { return 0; }