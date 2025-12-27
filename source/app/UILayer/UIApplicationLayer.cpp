#include "UIApplicationLayer.h"
#include <imgui/imgui.h>
#include <core/event/ApplicationEvent.h>
#include <iostream>

UIApplicationLayer::UIApplicationLayer(Core::LayerContext& ctx) : Core::IApplicationLayer(ctx), _sceneHierarchyPanel(*ctx.Get<Scene>().get()), _eventBus(*ctx.Get<Core::EventBus>().get())
{
	REGISTER_CALLBACK(_eventBus, Core::MouseDownEvent, OnMouseDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseUpEvent, OnMouseUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseMoveEvent, OnMouseMoveEvent);
	REGISTER_CALLBACK(_eventBus, Core::MouseScrollEvent, OnMouseScrollEvent);

	REGISTER_CALLBACK(_eventBus, Core::KeyDownEvent, OnKeyDownEvent);
	REGISTER_CALLBACK(_eventBus, Core::KeyUpEvent, OnKeyUpEvent);
	REGISTER_CALLBACK(_eventBus, Core::KeyCharacterEvent, OnKeyCharacterEvent);
}

void UIApplicationLayer::OnUpdate(const float deltaTime)
{
}

void UIApplicationLayer::OnRender()
{
	_sceneHierarchyPanel.OnRender();
}

bool UIApplicationLayer::OnMouseDownEvent(const Core::MouseDownEvent& e)
{
	if(!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse Button Down Event captured by UI Layer: " << e.identifier << std::endl;
	return true;
}

bool UIApplicationLayer::OnMouseUpEvent(const Core::MouseUpEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse button up event in UI: " << e.identifier << std::endl;
	return true;
}

bool UIApplicationLayer::OnMouseMoveEvent(const Core::MouseMoveEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse move event in UI: " << e.posX << ", " << e.posY << std::endl;
	return true;
}

bool UIApplicationLayer::OnMouseScrollEvent(const Core::MouseScrollEvent& e)
{
	if (!ImGui::GetIO().WantCaptureMouse)
		return false;

	std::cout << "Mouse scroll event in UI: " << e.scrollX << ", " << e.scrollY << std::endl;
	return true;
}

bool UIApplicationLayer::OnKeyDownEvent(const Core::KeyDownEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;

	std::cout << "Key down event in UI: " << e.key << ", repeated: " << e.repeated << std::endl;
	return true;
}

bool UIApplicationLayer::OnKeyUpEvent(const Core::KeyUpEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;

	std::cout << "Key up event in UI: " << e.key << std::endl;
	return true;
}

bool UIApplicationLayer::OnKeyCharacterEvent(const Core::KeyCharacterEvent& e)
{
	if (!ImGui::GetIO().WantCaptureKeyboard)
		return false;
	std::cout << "Key character event in UI: " << e.character << std::endl;
	return true;
}
