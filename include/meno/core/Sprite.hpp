//
// Created by 최상준 on 26. 9. 7..
//

#ifndef MENO_SPRITE_HPP
#define MENO_SPRITE_HPP

#include <meno/core/Component.hpp>
#include <meno/graphics/DrawParams.hpp>
#include <meno/graphics/Renderer.hpp>


namespace meno {

class Sprite : public Component {
public:
    explicit Sprite(GameObject* parent) { this->parent = parent; }
    void drawRect() {

    }
};

}


#endif // MENO_SPRITE_HPP
