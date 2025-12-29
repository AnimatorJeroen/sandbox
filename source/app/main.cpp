#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/UILayer/UIApplicationLayer.h"

int main() {

	Core::ApplicationSpecs specs {
		800,
		600,
		"Application Skeleton"
	};
	Core::Application app(specs);
	
	auto scene = std::make_shared<Scene>();
	app.GetContext().Register<Scene>(scene);
	app.PushLayer<UIApplicationLayer>();
	app.PushLayer<SceneApplicationLayer>();
    app.Run();

    return 0;
}