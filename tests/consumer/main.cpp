// 사용자 프로젝트가 meno 공개 헤더와 meno::meno 하나로 컴파일·링크되는지 확인한다.
// CI에는 디스플레이가 없으므로 인자를 줄 때만 창을 연다. 실행하지 않아도 링크는 검사된다.
#include <meno/Meno.hpp>

namespace {

class ConsumerGame final : public meno::Game {
public:
    ConsumerGame() : Game({.windowSize = {320u, 240u}, .title = "meno consumer"}) {}
};

} // namespace

int main(int argc, char**) {
    if (argc > 1) {
        ConsumerGame game;
        game.run();
    }
}