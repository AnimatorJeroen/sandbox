#pragma once
#include <memory>
#include <set>

class Scene;
namespace MyNamespace
{
	template<typename T1, typename T2>
	class Applicator;
}

class Panel_SceneHierarchy
{
public:
	explicit Panel_SceneHierarchy(Scene& scene, Core::Applicator<AppFieldTypes, AppComponentTypes>& applicator);
	~Panel_SceneHierarchy() = default;
	void Render();
	void SetContext(Scene& scene);
	const std::set<entt::entity>& GetSelectedEntities() const { return _selectedEntities; }
private:
	Scene* _scene;
	Core::Applicator<AppFieldTypes, AppComponentTypes>& _applicator;
	std::set<entt::entity> _selectedEntities;
	entt::entity _lastClickedEntity = entt::null;
};