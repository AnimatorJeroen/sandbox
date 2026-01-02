#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/editorLayer/EditorApplicationLayer.h"
#include "app/loggingLayer/LoggingApplicationLayer.h"
#include "app/sceneLayer/SceneManager.h"
#include <core/Logger.h>

#include <entt/entt.hpp>
#include <entt/meta/meta/meta.hpp>

#include <core/undo/Undo.h>
#include "app/sceneLayer/Scene.h"

static void reflect_types() {
    using namespace entt::literals;

    entt::meta_factory<Core::Vec3>()
        .data<&Core::Vec3::x>("x"_hs)
        .data<&Core::Vec3::y>("y"_hs)
        .data<&Core::Vec3::z>("z"_hs);
    entt::meta_factory<Core::Color>()
        .data<&Core::Color::r>("r"_hs)
        .data<&Core::Color::g>("g"_hs)
        .data<&Core::Color::b>("b"_hs)
        .data<&Core::Color::a>("a"_hs);
    entt::meta_factory<Scene>()
        .data<&Scene::sceneColor>("sceneColor"_hs);
}

int main() {

    reflect_types();

	Core::ApplicationSpecs specs {
		800,
		600,
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