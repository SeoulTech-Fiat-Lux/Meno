# GameObject 씬 예제

800 × 600 창의 가운데에 상자 스프라이트 하나를 표시하는 정적인 예제입니다.
텍스처를 코드에서 생성하므로 별도 에셋이나 실행 경로 설정이 필요하지 않습니다.

`Scene::createGameObject()`로 객체를 생성하고, 기본 컴포넌트인 `Transform`과
`Sprite`를 설정한 뒤 매 프레임 `GameObject::draw()`로 그립니다.
`Scene`은 객체를 소유하며, 현재 API에는 씬 전체를 자동으로 그리는 기능이 없습니다.

`MENO_BUILD_EXAMPLES=ON`으로 CMake를 구성한 뒤 `meno_example_gameobject_demo`
타깃을 빌드하고 실행하세요. 창의 닫기 버튼으로 종료합니다.
