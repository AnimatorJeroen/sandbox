#include "pch.h"
#include "Scene.h"
#include <random>
#include "core/UUID.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <filesystem>
#include <core/Logger.h>
#include "components/ComponentSerialization.h"

// C++17 compatible helper structs for Scene serialization
// Must be defined outside function scope for member templates to work
namespace {
	struct SceneSaveHelper {
		template<typename... Cs>
		static void save(std::ofstream& file, Core::Registry& registry, std::tuple<Cs...>*) {
			auto archive = Core::ArchiveHelpers::MakeFullSnapshot<Cs...>(registry);
			
			// Write entity count
			uint64_t entityCount = archive.entities.size();
			file.write(reinterpret_cast<const char*>(&entityCount), sizeof(entityCount));

			// Write all entity IDs
			file.write(reinterpret_cast<const char*>(archive.entities.data()),
				entityCount * sizeof(entt::entity));

			// Write each component storage
			writeStorages(file, archive, std::index_sequence_for<Cs...>{});
		}
		
		template<typename... Cs, std::size_t... Is>
		static void writeStorages(std::ofstream& file, 
			const Core::SelectionArchive<Cs...>& archive,
			std::index_sequence<Is...>) {
			// Use dummy array initialization trick for C++17 fold expression alternative
			int dummy[] = {0, (writeStorageAtIndex<Is>(file, archive), 0)...};
			(void)dummy; // Suppress unused variable warning
		}
		
		template<std::size_t I, typename... Cs>
		static void writeStorageAtIndex(std::ofstream& file, const Core::SelectionArchive<Cs...>& archive) {
			const auto& storage = std::get<I>(archive.storages);
			uint64_t count = storage.size();
			file.write(reinterpret_cast<const char*>(&count), sizeof(count));

			for (const auto& [entity, component] : storage) {
				file.write(reinterpret_cast<const char*>(&entity), sizeof(entity));
				// Use proper serialization that handles vectors
				ComponentSerialization::Serialize(file, component);
			}
		}
	};

	struct SceneLoadHelper {
		template<typename... Cs>
		static void load(std::ifstream& file, Core::Registry& registry, std::tuple<Cs...>*) {
			Core::SelectionArchive<Cs...> archive;

			// Read entity count
			uint64_t entityCount = 0;
			file.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount));

			// Read all entity IDs
			archive.entities.resize(entityCount);
			file.read(reinterpret_cast<char*>(archive.entities.data()),
				entityCount * sizeof(entt::entity));

			// Read each component storage
			readStorages(file, archive, std::index_sequence_for<Cs...>{});

			// Restore all entities (including the scene entity)
			Core::ArchiveHelpers::RestoreEntitiesAndComponents(registry, archive);
		}
		
		template<typename... Cs, std::size_t... Is>
		static void readStorages(std::ifstream& file, 
			Core::SelectionArchive<Cs...>& archive,
			std::index_sequence<Is...>) {
			// Use dummy array initialization trick for C++17 fold expression alternative
			int dummy[] = {0, (readStorageAtIndex<Is>(file, archive), 0)...};
			(void)dummy; // Suppress unused variable warning
		}
		
		template<std::size_t I, typename... Cs>
		static void readStorageAtIndex(std::ifstream& file, Core::SelectionArchive<Cs...>& archive) {
			auto& storage = std::get<I>(archive.storages);
			uint64_t count = 0;
			file.read(reinterpret_cast<char*>(&count), sizeof(count));

			using ComponentType = typename std::tuple_element<I, std::tuple<Cs...>>::type;
			storage.resize(count);
			for (uint64_t i = 0; i < count; ++i) {
				entt::entity entity;
				ComponentType component;
				file.read(reinterpret_cast<char*>(&entity), sizeof(entity));
				// Use proper deserialization that handles vectors
				ComponentSerialization::Deserialize(file, component);
				storage[i] = {entity, component};
			}
		}
	};
}

void Scene::UpdateCameraMatrices(uint32_t viewportWidth, uint32_t viewportHeight)
{
	auto& camera = GetActiveCamera();
	m_ViewMatrix = glm::lookAt(camera.position, camera.target, camera.up);
	float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	m_ProjectionMatrix = glm::perspective(glm::radians(camera.fov), aspect, camera.nearPlane, camera.farPlane);
}

void Scene::UpdateMatrices()
{
	// Find all root entities (entities with Transform but no Parent component)
	auto transformView = _registry.view<Transform, LocalToWorld>();
	
	for (auto entity : transformView) {
		// Only process entities without a parent - they are roots of hierarchy trees
		if (!_registry.all_of<Parent>(entity)) {
			// Update this root entity with identity parent matrix
			UpdateEntityHierarchyRecursive(entity, glm::mat4(1.0f));
		}
	}
}

void Scene::UpdateEntityHierarchyRecursive(entt::entity entity, const glm::mat4& parentWorldMatrix)
{
	// Get this entity's transform components
	auto& transform = _registry.get<Transform>(entity);
	auto& localToWorld = _registry.get<LocalToWorld>(entity);
	
	// Build local transform matrix from Transform component
	glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(transform.Rotation)));
	glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), transform.Position)
		* rotation
		* glm::scale(glm::mat4(1.0f), transform.Scale);
	
	// Compute world matrix by combining parent's world matrix with local matrix
	localToWorld.Value = parentWorldMatrix * localMatrix;
	
	// Recursively update all children with this entity's world matrix
	if (_registry.all_of<Children>(entity)) {
		const auto& children = _registry.get<Children>(entity);
		for (entt::entity childEntity : children.children) {
			// Only recurse if the child has transform components
			if (_registry.all_of<Transform, LocalToWorld>(childEntity)) {
				UpdateEntityHierarchyRecursive(childEntity, localToWorld.Value);
			}
		}
	}
}

void Scene::Draw(Core::DrawCommandRecorder& recorder)
{
	// Draw cubes for entities with MeshComponent
	int i = 0;
	for (auto entity : _registry.view<MeshComponent>()) {
		auto& localToWorld = _registry.get<LocalToWorld>(entity);
		recorder.Cube(localToWorld.Value, { 0.2f + i * 0.1f, 0.5f, 1.0f, 1.0f });
		i++;
	}

	// Draw skeleton bones as lines
	auto skeletonView = _registry.view<FBXSkeletonComponent>();
	for (auto skeletonEntity : skeletonView) {
		const auto& skeleton = _registry.get<FBXSkeletonComponent>(skeletonEntity);
		
		// Get the skeleton entity's world transform (if it has one)
		glm::mat4 skeletonWorldTransform = glm::mat4(1.0f);
		
		if (_registry.all_of<LocalToWorld>(skeletonEntity)) {
			skeletonWorldTransform = _registry.get<LocalToWorld>(skeletonEntity).Value;
		}
		skeletonWorldTransform = glm::scale(skeletonWorldTransform, vec3(.05f, .05f, .05f));
		
		 // Find root bones (bones with no parent)
		std::vector<int> rootBones;
		for (size_t boneIndex = 0; boneIndex < skeleton.bones.size(); boneIndex++) {
			const FBXBone& bone = *skeleton.bones[boneIndex];
			if (bone.parentIndex < 0) {
				rootBones.push_back(static_cast<int>(boneIndex));
			}
		}
		
		// Compute world transforms for all bones hierarchically using depth-first traversal
		// Each bone's world transform = parent's world transform * bone's animated local transform
		std::vector<glm::mat4> boneWorldTransforms(skeleton.bones.size());
		
		// Recursive function to update bone transforms in hierarchy order
		std::function<void(int, const glm::mat4&)> updateBoneHierarchy = 
			[&](int boneIndex, const glm::mat4& parentWorldTransform) {
			const FBXBone& bone = *skeleton.bones[boneIndex];
			
			// Compute this bone's world transform
			boneWorldTransforms[boneIndex] = parentWorldTransform * bone.localTransform;
			
			// Recursively update all children using stored childIndices
			for (int childIndex : bone.childIndices) {
				updateBoneHierarchy(childIndex, boneWorldTransforms[boneIndex]);
			}
		};
		
		// Start traversal from all root bones
		for (int rootIndex : rootBones) {
			updateBoneHierarchy(rootIndex, glm::mat4(1.0f));
		}
		
		// Draw lines from each bone to its parent
		for (size_t boneIndex = 0; boneIndex < skeleton.bones.size(); boneIndex++) {
			const FBXBone& bone = *skeleton.bones[boneIndex];
			
			//find parentBone


			if (bone.parentIndex >= 0 && bone.parentIndex < static_cast<int>(skeleton.bones.size())) {
				// Extract positions from world transforms and apply skeleton world transform
				glm::vec4 bonePosLocal =  (boneWorldTransforms[boneIndex]) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				glm::vec4 parentPosLocal =  (boneWorldTransforms[bone.parentIndex]) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

				glm::vec3 bonePos = glm::vec3(skeletonWorldTransform * bonePosLocal);
				glm::vec3 parentPos = glm::vec3(skeletonWorldTransform * parentPosLocal);

				// Draw line from parent to child bone
				recorder.Line(
					Core::Vec3{ parentPos.x, parentPos.y, parentPos.z },
					Core::Vec3{ bonePos.x, bonePos.y, bonePos.z },
					2.0f, 
					{ 1.0f, 0.8f, 0.0f, 1.0f }
				);
			}
		}
		
		// Draw small spheres at bone positions for visibility
		for (size_t boneIndex = 0; boneIndex < skeleton.bones.size(); boneIndex++) {
			glm::vec4 bonePosLocal = boneWorldTransforms[boneIndex] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec3 bonePos = glm::vec3(skeletonWorldTransform * bonePosLocal);
			
			// Draw a small circle at each bone position
			recorder.Circle(
				Core::Vec2{ bonePos.x, bonePos.y }, 
				0.05f, 
				true, 
				{ 1.0f, 0.5f, 0.0f, 1.0f }, 
				8, 
				1.0f
			);
		}
	}
}

void Scene::SetName(const std::string& name)
{
	data()._name = name;
}

const String64& Scene::GetName()
{
	return data()._name;
}

const std::string& Scene::GetFileName()
{
	return _fileName;
}

Entity Scene::CreateEntity(const bool addTransform)
{
	static int _entityCounter = 0;
	// Generate unique name
	std::string entityName = "Entity_" + std::to_string(_entityCounter++);
	return CreateEntity(entityName, addTransform);
}

Entity Scene::CreateEntity(const String64& name, const bool addTransform)
{
	entt::entity e = _registry.Create();
	// Add name component
	_registry.emplace<NameComponent>(e, name);

	if (addTransform)
	{
		_registry.emplace<Transform>(e, Transform());
		_registry.emplace<LocalToWorld>(e, LocalToWorld());
	}

	return Entity(e, &_registry);
}

Entity Scene::CreateCameraEntity()
{
	entt::entity e = _registry.Create();
	_registry.emplace<NameComponent>(e, "Camera");
	_registry.emplace<CameraComponent>(e);

	return Entity(e, &_registry);
}

std::set<Entity> Scene::GetAllEntities() const
{
	auto view = _registry.view<Core::UUID>();
	std::set<Entity> entities;
	for (auto e : view)
	{
		// Exclude the scene entity itself
		if (_sceneEntity != e)
			entities.insert(Entity(e, const_cast<Core::Registry*>(&_registry)));
	}
	return entities;
}

Entity Scene::GetEntity(uint64_t uuid) const
{
	// Search for an entity with the matching UUID
	auto view = _registry.view<Core::UUID>();
	for (auto e : view) {
		if (_registry.get<Core::UUID>(e).value == uuid) {
			return Entity(e, const_cast<Core::Registry*>(&_registry));
		}
	}
	
	// Return null entity if not found
	return Entity::Null();
}

inline CameraComponent& Scene::GetActiveCamera() { return _registry.get<CameraComponent>(_activeCamera); }

bool Scene::SaveToFile(const std::string& filepath) const
{
	try {
		// Create directory if it doesn't exist
		std::filesystem::path filePath(filepath);
		std::filesystem::create_directories(filePath.parent_path());

		std::ofstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERROR() << "Failed to open file for writing: " << filepath;
			return false;
		}

		// Write file format version
		uint32_t version = 1;
		file.write(reinterpret_cast<const char*>(&version), sizeof(version));

		// Create archive with all component types (including SceneData on _sceneEntity)
		// No need to sync camera - SceneData is already the source of truth
		SceneSaveHelper::save(file, const_cast<Core::Registry&>(_registry), static_cast<AppComponentTypes*>(nullptr));

		bool success = file.good();
		file.close();

		if (success) {
			LOG_DEBUG() << "Scene saved to: " << filepath;
		} else {
			LOG_ERROR() << "Error writing scene to file: " << filepath;
		}

		return success;
	}
	catch (const std::exception& e) {
		LOG_ERROR() << "Exception saving scene: " << e.what();
		return false;
	}
}

bool Scene::LoadFromFile(const std::string& filepath)
{
	try {
		std::ifstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			LOG_ERROR() << "Failed to open file for reading: " << filepath;
			return false;
		}

		// Read and validate file format version
		uint32_t version = 0;
		file.read(reinterpret_cast<char*>(&version), sizeof(version));
		if (version != 1) {
			LOG_ERROR() << "Unsupported scene file version: " << version;
			return false;
		 }

		// Clear existing scene
		_registry.clear();

		// Load entities and components using RestoreEntitiesAndComponents
		SceneLoadHelper::load(file, _registry, static_cast<AppComponentTypes*>(nullptr));

		// Find the scene entity by looking for SceneData component
		auto sceneDataView = _registry.view<SceneData>();
		if (sceneDataView.empty()) {
			LOG_ERROR() << "No SceneData component found in loaded scene";
			return false;
		}

		// Set _sceneEntity to the entity with SceneData
		_sceneEntity = sceneDataView.front();
		
		// Find and set the first camera entity as active camera
		auto cameraView = _registry.view<CameraComponent>();
		if (!cameraView.empty()) {
			_activeCamera = cameraView.front();
		}
		else {
			// No camera entity found, create a default one
			_activeCamera = CreateCameraEntity();
		}

		 // Camera settings are already in SceneData - just update matrices
		UpdateCameraMatrices(800, 600);

		RebuildChildrenForAllEntities();

		bool success = file.good();
		file.close();

		if (success) {
			LOG_DEBUG() << "Scene loaded from: " << filepath;
		} else {
			LOG_ERROR() << "Error reading scene from file: " << filepath;
		}

		return success;
	}
	catch (const std::exception& e) {
		LOG_ERROR() << "Exception loading scene: " << e.what();
		return false;
	}
}

void Scene::SetParent(Entity child, Entity parent)
{
	if (!child) {
		LOG_ERROR() << "SetParent: Invalid child entity";
		return;
	}

	if (!parent) {
		Entity oldParent = Entity::Null();
		if (child.HasComponent<Parent>()) {
			Parent& parentComp = child.GetComponent<Parent>();

			// Find the old parent entity by UUID to rebuild its children
			if (parentComp.HasParent()) {
				auto view = _registry.view<Core::UUID>();
				for (auto entity : view) {
					if (_registry.get<Core::UUID>(entity).value == parentComp.parentUUID.value) {
						oldParent = Entity(entity, &_registry);
						break;
					}
				}
			}
		}
		child.RemoveComponent<Parent>();
		if (oldParent)
			RebuildChildrenForEntity(oldParent);

		return;
	}

	// Get the child's UUID
	Core::UUID childUUID = child.UUID();
	Core::UUID parentUUID = parent.UUID();

	// Check if the child already has a parent
	Entity oldParent = Entity::Null();
	if (child.HasComponent<Parent>()) {
		Parent& parentComp = child.GetComponent<Parent>();
		
		// If setting the same parent, nothing to do
		if (parentComp.parentUUID.value == parentUUID.value) {
			return;
		}

		// Find the old parent entity by UUID to rebuild its children
		if (parentComp.HasParent()) {
			auto view = _registry.view<Core::UUID>();
			for (auto childEntity : view) {
				if (_registry.get<Core::UUID>(childEntity).value == parentComp.parentUUID.value) {
					oldParent = Entity(childEntity, &_registry);
					break;
				}
			}
		}

		// Update the parent UUID
		parentComp.parentUUID = parentUUID;
	}
	else {
		// Add new Parent component
		child.AddComponent<Parent>(parentUUID);
	}

	// Rebuild children for the old parent (if any)
	if (oldParent) {
		RebuildChildrenForEntity(oldParent);
	}

	// Rebuild children for the new parent
	RebuildChildrenForEntity(parent);

	LOG_DEBUG() << "Set parent: child UUID=" << childUUID.value << ", parent UUID=" << parentUUID.value;
}

void Scene::RebuildChildrenForEntity(Entity entity)
{
	if (!entity) {
		return;
	}

	Core::UUID entityUUID = entity.UUID();
	std::vector<entt::entity> childList;

	// Find all entities that have this entity as their parent
	auto view = _registry.view<Parent>();
	for (auto childEntity : view) {
		const Parent& parentComp = _registry.get<Parent>(childEntity);
		if (parentComp.parentUUID.value == entityUUID.value) {
			childList.push_back(childEntity);
		}
	}

	// Update or add Children component
	if (entity.HasComponent<Children>()) {
		entity.GetComponent<Children>().children = childList;
	}
	else if (!childList.empty()) {
		entity.AddComponent<Children>(childList);
	}
}

void Scene::RebuildSKeletonForEntity(Entity& skeletonEntity)
{
	auto& skeleton = skeletonEntity.GetComponent<FBXSkeletonComponent>();

	skeleton.bones.clear();
	auto children = skeletonEntity.GetAllSiblings();
	std::vector<Entity> boneEntities;
	for (const auto c : children)
	{
		if (!_registry.any_of<FBXBone>(c))
			continue;
		skeleton.bones.emplace_back(&_registry.get<FBXBone>(c));
		boneEntities.emplace_back(Entity(c, &_registry));
	}

	//now update indices for easy lookup
	for (size_t i = 0; i < skeleton.bones.size(); i++)
	{
		FBXBone& bone = *skeleton.bones[i];
		const Entity& boneEntity = boneEntities[i];
		Entity parent = boneEntity.GetParent();
		if (!parent)
			continue;
		bone.parentIndex = static_cast<int>(std::find(boneEntities.begin(), boneEntities.end(), parent) - boneEntities.begin());
		bone.parentIndex = bone.parentIndex == skeleton.bones.size() ? -1 : bone.parentIndex;
	}
	
	for (size_t i = 0; i < skeleton.bones.size(); i++)
	{
		FBXBone& bone = *skeleton.bones[i];
		// Find all children of this bone
		for (size_t j = 0; j < skeleton.bones.size(); j++)
		{
			if (skeleton.bones[j]->parentIndex == static_cast<int>(i))
			{
				bone.childIndices.push_back(static_cast<int>(j));
			}
		}
	}
}

void Scene::RebuildChildrenForAllEntities()
{
	auto view = _registry.view<Core::UUID>();
	for (auto entity : view) {
		RebuildChildrenForEntity(Entity(entity, &_registry));
	}

	auto skeletonView = _registry.view<FBXSkeletonComponent>();
	for (auto entity : skeletonView) {
		RebuildSKeletonForEntity(Entity(entity, &_registry));
	}

}

