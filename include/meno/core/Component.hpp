#ifndef MENO_COMPONENT_HPP
#define MENO_COMPONENT_HPP

namespace meno {
/// <summary>
/// 모든 component의 기본 구조체입니다. 본 구조체를 상속받아 새로운 component를 만들 수 있습니다.
/// </summary>
struct Component {
    virtual ~Component() = default;
};
} // namespace meno
#endif // MENO_COMPONENT_HPP