#pragma once
#include "Window.h"
#include <memory>
#include <vector>
#include "IApplicationLayer.h"
#include "event/ApplicationEvent.h"

namespace Core
{
	struct ApplicationSpecs
	{
		unsigned int windowWidth = 800;
		unsigned int windowHeight = 600;
		const char* windowTitle = "Application";
	};

	class Application
	{
		public:
			Application(const ApplicationSpecs& specs);
			~Application() = default;
			void Run();
			void Stop();
			void GetFrameBufferSize(int& width, int& height) const;
			bool OnWindowResizeEvent(const WindowResizeEvent& e);
			bool OnApplicationCloseEvent(const ApplicationCloseEvent& e);
			bool OnWindowCloseEvent(const WindowCloseEvent& e);
			template<typename TLayer>
			inline void PushLayer()
			{
				_applicationLayers.emplace_back(std::make_unique<TLayer>(_layerContext));
			}
			inline LayerContext& GetContext() { return _layerContext; }
		private:
			ApplicationSpecs _applicationSpecs;
			LayerContext _layerContext;
			std::shared_ptr<Window> _window;
			std::vector<std::unique_ptr<IApplicationLayer>> _applicationLayers;
			std::shared_ptr<EventBus> _eventBus = std::make_shared<EventBus>();


	};
}
