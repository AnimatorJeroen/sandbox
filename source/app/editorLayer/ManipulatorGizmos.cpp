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
	static std::unordered_map<entt::entity, glm::vec3> initialScales;
	static glm::vec3 initialCentroid{};
	static glm::vec3 initialAverageScale{};
	static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	if (!ImGuizmo::IsUsing())
	{
		operation = _imGuizmoOperation;
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

	// Calculate centroid (average position) of all selected objects
	glm::vec3 centroid(0.0f);
	glm::vec3 averageScale(0.0f);
	for (const auto& [entity, transform] : currentTransforms)
	{
		centroid += transform->Position;
		averageScale += transform->Scale;
	}
	centroid /= static_cast<float>(currentTransforms.size());
	averageScale /= static_cast<float>(currentTransforms.size());

	// Build model matrix at the centroid
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, centroid);
	modelMatrix = glm::scale(modelMatrix, averageScale);

	// Manipulate the gizmo
	ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
		operation, ImGuizmo::LOCAL, glm::value_ptr(modelMatrix));

	// Decompose the result to get new position and scale
	glm::vec3 newTranslation, newRotation, newScale;
	ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(modelMatrix),
		glm::value_ptr(newTranslation), glm::value_ptr(newRotation), glm::value_ptr(newScale));

	//Capture state and BeginUndo (aka OnMouseDown)
	if (ImGuizmo::IsUsing() && !imGuizmoActivate)
	{
		// Cache the transforms and initial centroid
		selectedTransforms = currentTransforms;
		initialCentroid = centroid;
		initialAverageScale = averageScale;

		// Store initial positions and scales for all selected entities
		initialPositions.clear();
		initialScales.clear();
		for (const auto& [entity, transform] : selectedTransforms)
		{
			initialPositions[entity] = transform->Position;
			initialScales[entity] = transform->Scale;
		}

		_editorContext.BeginUndo();
		imGuizmoActivate = true;

		// Capture initial state for undo
		for (const auto& transformPair : selectedTransforms)
		{
			auto entity = transformPair.first;
			auto& transform = *transformPair.second;

			if (operation == ImGuizmo::TRANSLATE)
			{
				_applicator.SetField(entity, "Transform.Position", transform.Position);
			}
			else if (operation == ImGuizmo::SCALE)
			{
				_applicator.SetField(entity, "Transform.Scale", transform.Scale);
			}
		}
	}

	//apply the transformation (aka OnMouseMove)
	if (imGuizmoActivate)
	{
		// Calculate deltas
		glm::vec3 translationDelta = newTranslation - initialCentroid;
		glm::vec3 scaleDelta = newScale / initialAverageScale;

		// Apply delta to all selected transforms
		for (const auto& transformPair : selectedTransforms)
		{
			auto entity = transformPair.first;
			auto& transform = *transformPair.second;

			if (operation == ImGuizmo::TRANSLATE)
			{
				// Apply translation delta to maintain relative positions
				transform.Position = initialPositions[entity] + translationDelta;
			}
			else if (operation == ImGuizmo::SCALE)
			{
				// Apply scale delta
				transform.Scale = initialScales[entity] * scaleDelta;
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

			if (operation == ImGuizmo::TRANSLATE)
			{
				_applicator.SetField(entity, "Transform.Position", transform.Position);
			}
			else if (operation == ImGuizmo::SCALE)
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