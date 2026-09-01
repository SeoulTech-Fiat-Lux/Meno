#ifndef MENO_CAMERA2D_HPP
#define MENO_CAMERA2D_HPP

#include <meno/math/Vec2.hpp>

namespace meno {

// 화면 상에 월드의 어느 영역을 얼만큼 비출지 기술하는 2D 카메라.
// SFML을 담지 않는 순수 데이터라 pimpl(pointer to implementation; sfml 구현부를 포인터로 가리키기)이 필요 없다.
// 값으로 자유롭게 복사하고 게임 쪽에서 들고 있다가, Renderer::setCamera로 넘기면 된다.
struct Camera2D {
    // 화면 중앙에 올 월드의 좌표.
    Vec2f center{0.f, 0.f};

    // 화면에 담을 월드 영역의 크기.
    // Camera2D::size == Vec2f{0.f, 0.f} -> 월드 유닛 == 1픽셀
    // 카메라의 사이즈가 {0, 0}이면 프레임 버퍼의 크기를 그대로 가져다 쓰기 때문에 월드 유닛의 크기가 1픽셀의 크기와 같아진다.
    Vec2f size{0.f, 0.f};

    // 화면의 배율(magnification).
    // 1보다 크면 확대(보이는 영역이 좁아짐), 작으면 축소.
    float zoom{1.f};

    // 화면의 각도(degree)
    // 0~360의 범위 안에서 시계 방향으로 회전.
    // e.g. Camera2D::rotation == 0 -> y축 위쪽, Camera2D::rotation == 90 -> x축 위쪽.
    float rotation{0.f};
};

} // namespace meno

#endif  // MENO_CAMERA2D_HPP