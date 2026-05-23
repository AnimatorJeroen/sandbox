#include "pch.h"
#include "Application.h"
#include "Window.h"
#include "event/EventBus.h"

#ifndef PLATFORM_WASM
#include "glew/glew.h"
#endif
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <core/Logger.h>
#ifdef PLATFORM_WASM
#include <emscripten.h>
#endif

namespace Core
{
	Application::Application(const ApplicationSpecs& specs) : _applicationSpecs(specs)
	{
		if (!glfwInit()) {
			LOG_ERROR() << "Failed to initialize GLFW";
			return;
		}
		
		_window = std::make_shared<Window>(_applicationSpecs.windowWidth, _applicationSpecs.windowHeight, _applicationSpecs.windowTitle, *_eventBus);

#ifndef PLATFORM_WASM
		if (glewInit() != GLEW_OK) {
			LOG_ERROR() << "Failed to initialize GLEW";
			return;
		}
#endif

#ifdef USE_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)_window->GetHandle(), true);
#ifdef PLATFORM_WASM
		ImGui_ImplOpenGL3_Init("#version 300 es");
#else
		ImGui_ImplOpenGL3_Init("#version 330");
#endif
#endif

		_layerContext.Register<EventBus>(_eventBus);
		_layerContext.Register<Window>(_window); // Register Window so layers can access itP

		//register callbacks
		REGISTER_CALLBACK((*_eventBus), WindowResizeEvent, OnWindowResizeEvent);
		REGISTER_CALLBACK((*_eventBus), WindowCloseEvent, OnWindowCloseEvent);
		REGISTER_CALLBACK((*_eventBus), WindowIconifiedEvent, OnWindowIconifiedEvent);
		REGISTER_CALLBACK((*_eventBus), WindowUnIconifiedEvent, OnWindowUnIconifiedEvent);
		// Note: ApplicationCloseEvent is registered in Run() after all layers are initialized

	}

	static bool shouldClose = false;
	void Application::Run()
	{
		// Register ApplicationCloseEvent callback last, so all layer callbacks are called first
		REGISTER_CALLBACK((*_eventBus), RequestApplicationCloseEvent, OnRequestApplicationCloseEvent);
		REGISTER_CALLBACK((*_eventBus), ApplicationCloseEvent, OnApplicationCloseEvent);

#ifdef PLATFORM_WASM
		emscripten_set_main_loop_arg([](void* arg) {
			static_cast<Application*>(arg)->Tick();
		}, this, 0, true);
#else
		while (!shouldClose)
		{
			Tick();
		}
		Stop();
#endif
	}

	void Application::Tick()
	{
		static float lastFrameTime = 0.0f;

		_window->PollEvents();
		_eventBus->HandleEvents();

		// Skip update and render if the application is paused
		if (_isPaused)
		{
			return;
		}

#ifdef USE_IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
#endif

		float currentFrameTime = static_cast<float>(glfwGetTime());
		float deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		for(auto it = _applicationLayers.rbegin(); it != _applicationLayers.rend(); ++it)
		{
			(*it)->OnUpdate(deltaTime);
		}

		for (auto it = _applicationLayers.rbegin(); it != _applicationLayers.rend(); ++it)
		{
			(*it)->OnRender();
		}

#ifdef USE_IMGUI
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

		_window->SwapBuffers();
	}

	void Application::Stop()
	{
		_window.reset();

#ifdef USE_IMGUI
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
#endif

		glfwTerminate();
	}

	void Application::GetFrameBufferSize(int& width, int& height) const
	{
		_window->GetFramebufferSize(width, height);
	}

	bool Application::OnWindowResizeEvent(const WindowResizeEvent& e)
	{
		//LOG_TRACE() << e.GetName() << ": " << e.Width << "x" << e.Height;
		return false;
	}

	bool Application::OnRequestApplicationCloseEvent(const RequestApplicationCloseEvent& e)
	{
		LOG_TRACE() << e.GetName() << " received.";
		_eventBus->PushEvent<ApplicationCloseEvent>(ApplicationCloseEvent());
		return true;
	}

	bool Application::OnApplicationCloseEvent(const ApplicationCloseEvent& e)
	{
		LOG_DEBUG() << e.GetName() << " received.";
		shouldClose = true;
		return true;
	}

	bool Application::OnWindowCloseEvent(const WindowCloseEvent& e)
	{
		LOG_DEBUG() << e.GetName() << " received.";
		if (e.windowHandle == _window->GetHandle())
		{
			_eventBus->PushEvent<RequestApplicationCloseEvent>(RequestApplicationCloseEvent());
		}
		return false;
	}

	bool Application::OnWindowIconifiedEvent(const WindowIconifiedEvent& e)
	{
		LOG_DEBUG() << e.GetName() << " received.";
		if (e.windowHandle == _window->GetHandle())
		{
			_isPaused = true;
			LOG_INFO() << "Application paused (window iconified).";
		}
		return false;
	}

	bool Application::OnWindowUnIconifiedEvent(const WindowUnIconifiedEvent& e)
	{
		LOG_DEBUG() << e.GetName() << " received.";
		if (e.windowHandle == _window->GetHandle())
		{
			_isPaused = false;
			LOG_INFO() << "Application unpaused (window restored).";
		}
		return false;
	}

}