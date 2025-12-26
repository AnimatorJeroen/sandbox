
#include "core/Application.h"
#include "app/applicationLayers/SceneApplicationLayer.h"
#include "app/applicationLayers/UIApplicationLayer.h"

int main() {

	Core::ApplicationSpecs specs {
		800,
		600,
		"Sandbox Application"
	};
	Core::Application app(specs);   
	app.PushLayer<SceneApplicationLayer>();
	app.PushLayer<UIApplicationLayer>();
    app.Run();

    return 0;
}