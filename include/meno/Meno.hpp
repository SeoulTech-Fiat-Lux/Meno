#pragma once

/// meno 통합 헤더.
///
/// 사용자는 이 한 줄로 공개 API 전체에 접근한다:
///
///     #include <meno/Meno.hpp>
///
/// 개별 헤더를 직접 include해도 된다. 컴파일 시간을 아끼려면 그쪽이 낫다.
///
/// 이 파일을 포함해 include/meno/ 아래 어떤 헤더도 <SFML/...>을 include하지
/// 않는다. 사용자 프로젝트는 SFML 헤더 경로 없이 meno만 링크하면 빌드된다.
/// tools/check_headers.py가 이 규칙을 검사한다.

// --- 값 타입 ---------------------------------------------------------------
#include <meno/math/Camera2D.hpp>
#include <meno/math/Color.hpp>
#include <meno/math/Rect.hpp>
#include <meno/math/Vec2.hpp>

// --- 그래픽 ---------------------------------------------------------------
#include <meno/graphics/DrawParams.hpp>
#include <meno/graphics/Font.hpp>
#include <meno/graphics/Renderer.hpp>
#include <meno/graphics/Texture.hpp>

// --- 코어 -----------------------------------------------------------------
#include <meno/core/Collider.hpp>
#include <meno/core/Component.hpp>
#include <meno/core/GameObject.hpp>
#include <meno/core/Window.hpp>

// 아직 없는 것 (담당자가 추가하면 여기에 한 줄씩 붙인다):
//   #include <meno/core/Application.hpp>
//   #include <meno/core/Time.hpp>
//   #include <meno/scene/Scene.hpp>
//   #include <meno/scene/GameObject.hpp>
//   #include <meno/collision/Collision.hpp>
//   #include <meno/input/Input.hpp>
//   #include <meno/audio/Audio.hpp>
//   #include <meno/resources/ResourceManager.hpp>
