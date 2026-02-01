#include "pch.h"
#include "EditorApplicationLayer.h"
#include <ImGuizmo/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

void EditorApplicationLayer::RenderImGuizmo()
{
	auto scene = _sceneManager->GetActiveScene();
	if (!scene)
		return;
	auto& registry = scene->GetRegistry();
	auto selectedEntities = _editorContext.GetSelectedEntities();

	if (selectedEntities.empty())
		return;

	// Setup ImGuizmo
	int windowWidth = 1280, windowHeight = 720;
	ImGuizmo::SetRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight));

	// Get camera matrices  
	auto& cameraComponent = scene->GetActiveCamera();
	const glm::mat4 viewMatrix = glm::lookAt(cameraComponent.position, cameraComponent.target, cameraComponent.up);
	const glm::mat4 projectionMatrix = glm::perspective(glm::radians(cameraComponent.fov),
		static_cast<float>(windowWidth) / static_cast<float>(windowHeight),
		cameraComponent.nearPlane, cameraComponent.farPlane);

	static bool imGuizmoActivate = false;
	static std::vector<std::pair<entt::entity, Transform*>> selectedTransforms;
	static std::unordered_map<entt::entity, glm::vec3> initialPositions;
	static std::unordered_map<entt::entity, glm::vec3> initialRotations;
	static std::unordered_map<entt::entity, glm::vec3> initialScales;
	static glm::vec3 initialCentroid{};
	static glm::vec3 initialAverageScale{};
	static glm::vec3 initialAverageRotation{};
	static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	static ImGuizmo::MODE mode = ImGuizmo::WORLD;
	if (!ImGuizmo::IsUsing())
	{
		operation = _imGuizmoOperation;
		mode = _imGuizmoMode;
	}

	// Collect all transforms from selected entities
	std::vector<std::pair<entt::entity, Transform*>> currentTransforms;
	for (const auto& entity : selectedEntities)
	{
		if (registry.any_of<Transform>(entity))
		{
			auto& transform = registry.get<Transform>(entity);
			currentTransforms.push_back({ entity, &transform });
		}
	}

	if (currentTransforms.empty())
		return;

	bool isWorldMultiSelect = (mode == ImGuizmo::WORLD) && (currentTransforms.size() > 1);

	// Calculate centroid (average position) of all selected objects
	glm::vec3 centroid(0.0f);
	glm::vec3 averageScale(0.0f);
	glm::vec3 averageRotation(0.0f);
	for (const auto& [entity, transform] : currentTransforms)
	{
		centroid += transform->Position;
		averageScale += transform->Scale;
		averageRotation += transform->Rotation;
	}
	centroid /= static_cast<float>(currentTransforms.size());
	averageScale /= static_cast<float>(currentTransforms.size());
	averageRotation /= static_cast<float>(currentTransforms.size());

	if (!imGuizmoActivate)
	{
		initialCentroid = centroid;
		initialAverageScale = averageScale;
		initialAverageRotation = averageRotation;
	}

	// Build model matrix at the centroid
	glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(averageRotation)));
	mat4 manipulatorMatrix = glm::translate(glm::mat4(1.0f), operation == ImGuizmo::TRANSLATE ? centroid : initialCentroid)
		* rotation
		* glm::scale(glm::mat4(1.0f), averageScale);

	mat4 transformedManipulatorMatrix = manipulatorMatrix;

	glm::mat4 deltaMatrix;
	// Manipulate the gizmo
	ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
		operation, mode, glm::value_ptr(transformedManipulatorMatrix), glm::value_ptr(deltaMatrix));

	// Decompose the result to get new position and scale
	glm::vec3 deltaTranslation, deltaRotation, deltaScale;
	ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(deltaMatrix),
		glm::value_ptr(deltaTranslation), glm::value_ptr(deltaRotation), glm::value_ptr(deltaScale));

	//Capture state and BeginUndo (aka OnMouseDown)
	if (ImGuizmo::IsUsing() && !imGuizmoActivate)
	{
		// Cache the transforms and initial centroid
		selectedTransforms = currentTransforms;

		// Store initial positions and scales for all selected entities
		initialPositions.clear();
		initialScales.clear();
		initialRotations.clear();
		for (const auto& [entity, transform] : selectedTransforms)
		{
			initialPositions[entity] = transform->Position;
			initialScales[entity] = transform->Scale;
			initialRotations[entity] = transform->Rotation;
		}

		_editorContext.BeginUndo();
		imGuizmoActivate = true;

		// Capture initial state for undo
		for (const auto& transformPair : selectedTransforms)
		{
			auto entity = transformPair.first;
			auto& transform = *transformPair.second;

			if (operation == ImGuizmo::TRANSLATE || isWorldMultiSelect)
			{
				_applicator.SetField(entity, "Transform.Position", transform.Position);
			}
			if (operation == ImGuizmo::ROTATE || isWorldMultiSelect)
			{
				_applicator.SetField(entity, "Transform.Rotation", transform.Rotation);
			}
			if (operation == ImGuizmo::SCALE || isWorldMultiSelect)
			{
				_applicator.SetField(entity, "Transform.Scale", transform.Scale);
			}
		}
	}

	//apply the transformation (aka OnMouseMove)
	if (imGuizmoActivate)
	{
		// Apply delta to all selected transforms
		for (const auto& transformPair : selectedTransforms)
		{
			auto entity = transformPair.first;
			auto& transform = *transformPair.second;

			if (isWorldMultiSelect &&
				(operation == ImGuizmo::TRANSLATE || operation == ImGuizmo::ROTATE || operation == ImGuizmo::SCALE))
			{
				mat4 originalMatrix = transform.GetTransform();
				mat4 m = originalMatrix;

				//bring to parent space
				//m = glm::inverse(manipulatorMatrix) * m;
				//now apply delta
				//m = deltaMatrix * m;

				//bring back to world space (apply parent space)
				//m = manipulatorMatrix * m;

				m = deltaMatrix * m;

				//mat4 transformed_modelMatrixInParentSpace = glm::inverse(transformedManipulatorMatrix) * modelMatrix;

				//mat4 thisDeltaMatrixInParentSpace = glm::inverse(modelMatrixInParentSpace) * transformed_modelMatrixInParentSpace;

				//mat4 thisDeltaMatrix = glm::inverse(originalMatrix) * m;

				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(m),
					glm::value_ptr(deltaTranslation), glm::value_ptr(deltaRotation), glm::value_ptr(deltaScale));

				transform.Position = deltaTranslation;
				transform.Rotation = deltaRotation;
				transform.Scale = deltaScale;
			}
			else if (operation == ImGuizmo::TRANSLATE)
			{
				// Apply translation delta to maintain relative positions
				transform.Position += deltaTranslation;
			}
			else if (operation == ImGuizmo::ROTATE)
			{
				// Apply translation delta to maintain relative positions
				transform.Rotation += deltaRotation;
			}
			else if (operation == ImGuizmo::SCALE)
			{
				// Apply scale delta
				transform.Scale *= deltaScale;
			}
		}
	}

	//capture end state and EndUndo (aka OnMouseUp)
	if (imGuizmoActivate && !ImGuizmo::IsUsing())
	{
		// Capture final state for undo
		for (const auto& transformPair : selectedTransforms)
		{
			auto entity = transformPair.first;
			auto& transform = *transformPair.second;

			if (operation == ImGuizmo::TRANSLATE || isWorldMultiSelect)
			{
				_applicator.SetField(entity, "Transform.Position", transform.Position);
			}
			if (operation == ImGuizmo::ROTATE || isWorldMultiSelect)
			{
				_applicator.SetField(entity, "Transform.Rotation", transform.Rotation);
			}
			if (operation == ImGuizmo::SCALE || isWorldMultiSelect)
			{
				_applicator.SetField(entity, "Transform.Scale", transform.Scale);
			}
		}
		_editorContext.EndUndo();
		imGuizmoActivate = false;
		selectedTransforms.clear();
		initialPositions.clear();
		initialScales.clear();
	}
}