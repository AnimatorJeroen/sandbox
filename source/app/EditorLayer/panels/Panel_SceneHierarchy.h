#pragma once
#include <memory>
#include <set>

class Scene;
class EditorContext;

namespace Core {
	template<typename T1, typename T2>
	class Applicator;
}

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& scene, EditorContext& editorContext);
	~Panel_SceneHierarchy() = default;
	void Render();
	void SetContext(Scene& scene);
private:
	Scene* _scene;
	EditorContext& _editorContext;
	entt::entity _lastClickedEntity = entt::null;
};