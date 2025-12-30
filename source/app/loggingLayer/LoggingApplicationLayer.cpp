#include "LoggingApplicationLayer.h"
#include <iostream>
#include <imgui/imgui.h>
#include <core/Logger.h>

LoggingApplicationLayer::LoggingApplicationLayer(Core::LayerContext& ctx) 
	: Core::IApplicationLayer(ctx),
	_eventBus(*ctx.Get<Core::EventBus>().get())
{
	// Enable GUI logging
	Core::Log::EnableGuiLogging(true);
	Core::Log::SetMaxGuiLogs(1000);
}

void LoggingApplicationLayer::OnUpdate(const float deltaTime)
{
}

void LoggingApplicationLayer::OnRender()
{
	ImGui::Begin("Logger", nullptr, ImGuiWindowFlags_MenuBar);
	
	// Menu bar for controls
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &_autoScroll);
			ImGui::Separator();
			ImGui::Text("Filter Levels:");
			ImGui::Checkbox("Trace", &_showTrace);
			ImGui::Checkbox("Debug", &_showDebug);
			ImGui::Checkbox("Info", &_showInfo);
			ImGui::Checkbox("Warn", &_showWarn);
			ImGui::Checkbox("Error", &_showError);
			ImGui::Checkbox("Critical", &_showCritical);
			ImGui::EndMenu();
		}
		
		if (ImGui::Button("Clear"))
		{
			Core::Log::ClearGuiLogs();
			_previousLogCount = 0;
		}
		
		ImGui::EndMenuBar();
	}
	
	// Get logs from logger
	auto logs = Core::Log::GetGuiLogs();
	
	ImGui::Text("Total Logs: %zu", logs.size());
	ImGui::Separator();
	
	// Check if new logs were added
	bool hasNewLogs = logs.size() > _previousLogCount;
	_previousLogCount = logs.size();
	
	// Event list in a scrollable region
	ImGui::BeginChild("LogList", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
	
	for (const auto& entry : logs)
	{
		// Filter by log level
		bool shouldShow = false;
		switch (entry.level)
		{
		case Core::Log::Level::Trace:    shouldShow = _showTrace; break;
		case Core::Log::Level::Debug:    shouldShow = _showDebug; break;
		case Core::Log::Level::Info:     shouldShow = _showInfo; break;
		case Core::Log::Level::Warn:     shouldShow = _showWarn; break;
		case Core::Log::Level::Error:    shouldShow = _showError; break;
		case Core::Log::Level::Critical: shouldShow = _showCritical; break;
		}
		
		if (!shouldShow)
			continue;
		
		// Color code different log levels
		ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White default
		switch (entry.level)
		{
		case Core::Log::Level::Trace:    color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); break; // Gray
		case Core::Log::Level::Debug:    color = ImVec4(0.5f, 0.8f, 1.0f, 1.0f); break; // Light blue
		case Core::Log::Level::Info:     color = ImVec4(0.6f, 1.0f, 0.6f, 1.0f); break; // Green
		case Core::Log::Level::Warn:     color = ImVec4(1.0f, 1.0f, 0.4f, 1.0f); break; // Yellow
		case Core::Log::Level::Error:    color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break; // Red
		case Core::Log::Level::Critical: color = ImVec4(1.0f, 0.0f, 1.0f, 1.0f); break; // Magenta
		}
		
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		
		// Format: [timestamp] [LEVEL] message (function file:line)
		const char* levelName = "";
		switch (entry.level)
		{
		case Core::Log::Level::Trace:    levelName = "TRACE"; break;
		case Core::Log::Level::Debug:    levelName = "DEBUG"; break;
		case Core::Log::Level::Info:     levelName = "INFO"; break;
		case Core::Log::Level::Warn:     levelName = "WARN"; break;
		case Core::Log::Level::Error:    levelName = "ERROR"; break;
		case Core::Log::Level::Critical: levelName = "CRIT"; break;
		}
		
		ImGui::Text("[%s] [%s] %s", entry.timestamp.c_str(), levelName, entry.message.c_str());
		
		// Show details on hover
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Function: %s", entry.func.c_str());
			ImGui::Text("File: %s:%d", entry.file.c_str(), entry.line);
			ImGui::EndTooltip();
		}
		
		ImGui::PopStyleColor();
	}
	
	// Auto-scroll to bottom only when new logs are added
	if (_autoScroll && hasNewLogs)
	{
		ImGui::SetScrollHereY(1.0f);
	}
	
	ImGui::EndChild();
	ImGui::End();
}
