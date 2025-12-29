#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/UILayer/UIApplicationLayer.h"

//forward declare
class SceneManager;

int main() {

	Core::ApplicationSpecs specs {
		800,
		600,
		"Application Skeleton"
	};
	Core::Application app(specs);

	auto sceneManager = std::make_shared<SceneManager>();
	app.GetContext().Register<SceneManager>(sceneManager);
	
	app.PushLayer<UIApplicationLayer>();
	app.PushLayer<SceneApplicationLayer>();
    app.Run();

    return 0;
}