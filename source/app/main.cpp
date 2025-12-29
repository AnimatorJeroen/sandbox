#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/editorLayer/EditorApplicationLayer.h"
#include "app/loggingLayer/LoggingApplicationLayer.h"
#include "app/sceneLayer/SceneManager.h"

int main() {

	Core::ApplicationSpecs specs {
		800,
		600,
		"Application Skeleton"
	};
	Core::Application app(specs);

	auto sceneManager = std::make_shared<SceneManager>(*app.GetContext().Get<Core::EventBus>());
	app.GetContext().Register<SceneManager>(sceneManager);
	
	app.PushLayer<LoggingApplicationLayer>();
	app.PushLayer<EditorApplicationLayer>();
	app.PushLayer<SceneApplicationLayer>();
    app.Run();

    return 0;
}