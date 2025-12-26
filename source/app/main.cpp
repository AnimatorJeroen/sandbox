
#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/UILayer/UIApplicationLayer.h"

int main() {

	Core::ApplicationSpecs specs {
		800,
		600,
		"Sandbox Application"
	};
	Core::Application app(specs);   
	app.GetContext().Register<Scene>(std::make_shared<Scene>());
	app.PushLayer<SceneApplicationLayer>();
	app.PushLayer<UIApplicationLayer>();
    app.Run();

    return 0;
}