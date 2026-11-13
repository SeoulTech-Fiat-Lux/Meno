#ifndef MENO_HPP
#define MENO_HPP

// include/meno/ 아래 어떤 헤더도 <SFML/...>을 include하지 않는다.
// tools/check_headers.py가 이 규칙을 검사한다.
/// \file
/// meno 통합 헤더입니다.
///
/// 이 한 줄로 공개 API 전체를 씁니다:
///
/// \code{.cpp}
/// #include <meno/Meno.hpp>
/// \endcode
///
/// 개별 헤더를 직접 include해도 되며, 컴파일 시간을 아끼려면 그쪽이 낫습니다. 공개 헤더는 SFML
/// 헤더를 include하지 않으므로, 사용자 프로젝트는 SFML 헤더 경로 없이 meno만 링크하면
/// 빌드됩니다.

#include <meno/math/Angle.hpp>    // IWYU pragma: export
#include <meno/math/Camera2D.hpp> // IWYU pragma: export
#include <meno/math/Color.hpp>    // IWYU pragma: export
#include <meno/math/Rect.hpp>     // IWYU pragma: export
#include <meno/math/Vec2.hpp>     // IWYU pragma: export

#include <meno/audio/Music.hpp>       // IWYU pragma: export
#include <meno/audio/Sound.hpp>       // IWYU pragma: export
#include <meno/audio/SoundBuffer.hpp> // IWYU pragma: export

#include <meno/graphics/DrawParams.hpp> // IWYU pragma: export
#include <meno/graphics/Font.hpp>       // IWYU pragma: export
#include <meno/graphics/Renderer.hpp>   // IWYU pragma: export
#include <meno/graphics/Texture.hpp>    // IWYU pragma: export

#include <meno/resources/ResourceManager.hpp> // IWYU pragma: export

#include <meno/core/Application.hpp> // IWYU pragma: export
#include <meno/core/Clock.hpp>       // IWYU pragma: export
#include <meno/core/Time.hpp>        // IWYU pragma: export

#include <meno/window/Window.hpp> // IWYU pragma: export

#include <meno/input/Event.hpp>       // IWYU pragma: export
#include <meno/input/Input.hpp>       // IWYU pragma: export
#include <meno/input/Key.hpp>         // IWYU pragma: export
#include <meno/input/MouseButton.hpp> // IWYU pragma: export

#include <meno/scene/AudioSource.hpp>  // IWYU pragma: export
#include <meno/scene/Collider.hpp>     // IWYU pragma: export
#include <meno/scene/Component.hpp>    // IWYU pragma: export
#include <meno/scene/GameObject.hpp>   // IWYU pragma: export
#include <meno/scene/Scene.hpp>        // IWYU pragma: export
#include <meno/scene/SceneManager.hpp> // IWYU pragma: export
#include <meno/scene/Sprite.hpp>       // IWYU pragma: export
#include <meno/scene/Transform.hpp>    // IWYU pragma: export

#include <meno/app/Game.hpp> // IWYU pragma: export

#endif // MENO_HPP
