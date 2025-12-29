#include "LoggingApplicationLayer.h"
#include <iostream>
#include <imgui/imgui.h>

LoggingApplicationLayer::LoggingApplicationLayer(Core::LayerContext& ctx) 
	: Core::IApplicationLayer(ctx),
	_eventBus(*ctx.Get<Core::EventBus>().get())
{
}

void LoggingApplicationLayer::OnUpdate(const float deltaTime)
{
}

void LoggingApplicationLayer::OnRender()
{
	ImGui::Begin("Logger");
	ImGui::Text("Events");



	ImGui::End();
}
