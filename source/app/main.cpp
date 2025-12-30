#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/editorLayer/EditorApplicationLayer.h"
#include "app/loggingLayer/LoggingApplicationLayer.h"
#include "app/sceneLayer/SceneManager.h"
#include <core/Logger.h>

int main() {

	Core::ApplicationSpecs specs {
		800,
		600,
		"Application Skeleton"
	};
	Core::Application app(specs);


	Core::Log::SetLevel(Core::Log::Level::Trace);
	Core::Log::EnableConsole(true);

	auto sceneManager = std::make_shared<SceneManager>(*app.GetContext().Get<Core::EventBus>());
	app.GetContext().Register<SceneManager>(sceneManager);
	
	app.PushLayer<LoggingApplicationLayer>();
	app.PushLayer<EditorApplicationLayer>();
	app.PushLayer<SceneApplicationLayer>();
    app.Run();

    return 0;
}