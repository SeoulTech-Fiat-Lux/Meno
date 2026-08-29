//
// Created by 최상준 on 26. 8. 6..
//

#include <meno/core/GameObject.hpp>

template <typename T, typename... Args>
    requires std::derived_from<T, Component>
T& meno::GameObject::addComponent(Args&&... args) {
    
}

template<typename T> 
T& meno::GameObject::getComponent() noexcept {
    auto it = components_.find(std::type_index(typeid(T)));
    if (it != components_.end()) {
        return *static_cast<T*>(it->second.get());
    }
    throw std::runtime_error("Component not found");
}