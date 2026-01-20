#include "pch.h"
#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/editorLayer/EditorApplicationLayer.h"
#include "app/loggingLayer/LoggingApplicationLayer.h"
#include "app/sceneLayer/SceneManager.h"
#include <core/Logger.h>

#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

#include "app/sceneLayer/Scene.h"
#include "app/sceneLayer/types/ReflectTypes.h"

int main() {

    ReflectTypes();

	Core::ApplicationSpecs specs {
		1280,
		720,
		"Application Skeleton"
	};
	Core::Application app(specs);


	Core::Log::SetLevel(Core::Log::Level::Debug);
	Core::Log::EnableConsole(false);

	auto sceneManager = std::make_shared<SceneManager>(*app.GetContext().Get<Core::EventBus>());
	app.GetContext().Register<SceneManager>(sceneManager);
	
	app.PushLayer<LoggingApplicationLayer>();
	app.PushLayer<EditorApplicationLayer>();
	app.PushLayer<SceneApplicationLayer>();
    app.Run();

    return 0;
}