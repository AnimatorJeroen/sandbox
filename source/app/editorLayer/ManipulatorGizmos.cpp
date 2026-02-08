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
	auto& selectedEntities = _editorContext.GetSelectedEntities();

	if (selectedEntities.empty())
		return;

	// Setup ImGuizmo
	ImGuiIO& io = ImGui::GetIO();
	float windowWidth = io.DisplaySize.x, windowHeight = io.DisplaySize.y;
	if(windowWidth <= 1.f || windowHeight <= 1.f || isnan(windowWidth))
		return;

	ImGuizmo::SetRect(0, 0, windowWidth, windowHeight);

	// Get camera matrices  
	auto& cameraComponent = scene->GetActiveCamera();
	const glm::mat4 viewMatrix = glm::lookAt(cameraComponent.position, cameraComponent.target, cameraComponent.up);
	const glm::mat4 projectionMatrix = glm::perspective(glm::radians(cameraComponent.fov),
		windowWidth / windowHeight,
		cameraComponent.nearPlane, cameraComponent.farPlane);

	static bool imGuizmoActivate = false;
	static std::vector<std::pair<Entity, TransformBundle>> selectedTransforms;
	static glm::vec3 initialCentroid{};
	static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	static ImGuizmo::MODE mode = ImGuizmo::WORLD;
	if (!ImGuizmo::IsUsing())
	{
		operation = _editorContext.GetImGuizmoOperation();
		mode = _editorContext.GetImGuizmoMode();
	}

	// Collect all transforms from selected entities
	std::vector<std::pair<Entity, TransformBundle>> currentTransforms;
	for (auto& entity : selectedEntities)
	{
		if (entity.HasComponent<Transform>())
		{
			currentTransforms.emplace_back(entity, const_cast<Entity&>(entity).GetTransformBundle());
		}
	}
	//remove any entities that are rt_children of other selected entities
	{
		std::vector<std::pair<Entity, TransformBundle>> filteredTransforms;
		for (auto& [entity, transform] : currentTransforms)
		{
			bool isChildOfSelected = false;
			Entity parent = entity.GetParent();
			while (parent)
			{
				if (std::any_of(selectedEntities.begin(), selectedEntities.end(),
					[&parent](const Entity& e) { return e.GetHandle() == parent.GetHandle(); }))
				{
					isChildOfSelected = true;
					break;
				}
				parent = parent.GetParent();
			}
			if (!isChildOfSelected)
			{
				filteredTransforms.emplace_back(entity, transform);
			}
		}
		currentTransforms = std::move(filteredTransforms);
	}

	if (currentTransforms.empty())
		return;

	bool multiSelect = (currentTransforms.size() > 1);

	glm::vec3 centroid(0.0f);
	glm::vec3 averageScale(0.0f);
	glm::vec3 averageRotation(0.0f);
	for (auto& [entity, transform] : currentTransforms)
	{
		centroid += transform.worldPosition;
		averageScale += transform.worldScale;
		averageRotation += transform.worldRotation;
	}
	centroid /= static_cast<float>(currentTransforms.size());
	averageScale /= static_cast<float>(currentTransforms.size());
	averageRotation /= static_cast<float>(currentTransforms.size());

	if (!imGuizmoActivate)
		initialCentroid = centroid;

	if(mode == ImGuizmo::WORLD && multiSelect && operation == ImGuizmo::SCALE)
		averageRotation = vec3();

	// Build model matrix at the centroid
	glm::mat4 rotation = glm::toMat4(glm::quat(glm::radians(averageRotation)));
	mat4 manipulatorMatrix = glm::translate(glm::mat4(1.0f), operation == ImGuizmo::TRANSLATE ? centroid : initialCentroid)
		* rotation
		* glm::scale(glm::mat4(1.0f), averageScale);

	mat4 transformedManipulatorMatrix = manipulatorMatrix;

	// Manipulate the gizmo
	glm::mat4 deltaMatrix;
	ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
		operation, mode, glm::value_ptr(transformedManipulatorMatrix), glm::value_ptr(deltaMatrix));

	//Capture state and BeginUndo (aka OnMouseDown)
	if (ImGuizmo::IsUsing() && !imGuizmoActivate)
	{
		// Cache the transforms and initial centroid
		selectedTransforms.clear();
		for (auto& currentTransform : currentTransforms)
		{
			selectedTransforms.emplace_back(currentTransform);
		}
		_editorContext.BeginUndo();
		imGuizmoActivate = true;

		// Capture initial state for undo
		for (const auto& transformPair : selectedTransforms)
		{
			auto entity = transformPair.first;
			auto& transform = transformPair.second;

			if (operation == ImGuizmo::TRANSLATE || multiSelect)
			{
				_applicator.SetField(entity.GetHandle(), "Transform.Position", transform.Position);
			}
			if (operation == ImGuizmo::ROTATE || multiSelect)
			{
				_applicator.SetField(entity.GetHandle(), "Transform.Rotation", transform.Rotation);
			}
			if (operation == ImGuizmo::SCALE || multiSelect)
			{
				_applicator.SetField(entity.GetHandle(), "Transform.Scale", transform.Scale);
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
			auto& transform = transformPair.second;

			if (mode == ImGuizmo::WORLD || multiSelect &&
				(operation == ImGuizmo::TRANSLATE || operation == ImGuizmo::ROTATE || operation == ImGuizmo::SCALE))
			{
				mat4 m = transform.LocalToWorld;
				if (operation == ImGuizmo::SCALE)
				{
					//bring to parent space
					m = glm::inverse(manipulatorMatrix) * m;
					//now apply delta
					m = deltaMatrix * m;

					//bring back to world space (apply parent space)
					m = manipulatorMatrix * m;
				}
				else
				{
					m = deltaMatrix * m;
				}

				vec3 translation, rotation, scale;
				glm::quat orientation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(m, scale, orientation, translation, skew, perspective);
				rotation = glm::degrees(glm::eulerAngles(orientation));

				transform.Position = translation;
				transform.Rotation = rotation;
				transform.Scale = scale;
			}
			else
			{
				vec3 translation, rotation, scale;
				glm::quat orientation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(transformedManipulatorMatrix, scale, orientation, translation, skew, perspective);
				rotation = glm::degrees(glm::eulerAngles(orientation));


				if (operation == ImGuizmo::TRANSLATE)
				{
					glm::mat4 parentSpace = glm::inverse(transform.GetParentMatrix());
					transform.Position = parentSpace * glm::vec4(translation, 1);
				}
				else if (operation == ImGuizmo::ROTATE)
				{
					glm::mat4 parentMatrix = transform.GetParentMatrix();
					glm::quat parentRotation;
					glm::vec3 parentPos, parentScale, parentSkew;
					glm::vec4 parentPerspective;
					glm::decompose(parentMatrix, parentScale, parentRotation, parentPos, parentSkew, parentPerspective);

					glm::quat localRotationQuat = glm::inverse(parentRotation) * orientation;
					transform.Rotation = glm::degrees(glm::eulerAngles(localRotationQuat));
				}
				else if (operation == ImGuizmo::SCALE)
				{
					glm::mat4 parentMatrix = transform.GetParentMatrix();
					glm::quat parentRotation;
					glm::vec3 parentPos, parentScale, parentSkew;
					glm::vec4 parentPerspective;
					glm::decompose(parentMatrix, parentScale, parentRotation, parentPos, parentSkew, parentPerspective);

					transform.Scale = scale / parentScale;
				}

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
			auto& transform = transformPair.second;

			if (operation == ImGuizmo::TRANSLATE || multiSelect)
			{
				_applicator.SetField(entity.GetHandle(), "Transform.Position", transform.Position);
			}
			if (operation == ImGuizmo::ROTATE || multiSelect)
			{
				_applicator.SetField(entity.GetHandle(), "Transform.Rotation", transform.Rotation);
			}
			if (operation == ImGuizmo::SCALE || multiSelect)
			{
				_applicator.SetField(entity.GetHandle(), "Transform.Scale", transform.Scale);
			}
		}
		_editorContext.EndUndo();
		imGuizmoActivate = false;
		selectedTransforms.clear();
	}
}