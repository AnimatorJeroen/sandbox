#include "pch.h"
#include "Application.h"
#include "Window.h"
#include "event/EventBus.h"

#include "glew/glew.h"
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <core/Logger.h>

namespace Core
{
	Application::Application(const ApplicationSpecs& specs) : _applicationSpecs(specs)
	{
		if (!glfwInit()) {
			LOG_ERROR() << "Failed to initialize GLFW";
			return;
		}
		
		_window = std::make_shared<Window>(_applicationSpecs.windowWidth, _applicationSpecs.windowHeight, _applicationSpecs.windowTitle, *_eventBus);

		if (glewInit() != GLEW_OK) {
			LOG_ERROR() << "Failed to initialize GLEW";
			return;
		}

#ifdef USE_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)_window->GetHandle(), true);
		ImGui_ImplOpenGL3_Init("#version 330");
#endif

		_layerContext.Register<EventBus>(_eventBus);
		_layerContext.Register<Window>(_window); // Register Window so layers can access itP

		//register callbacks
		REGISTER_CALLBACK((*_eventBus), WindowResizeEvent, OnWindowResizeEvent);
		REGISTER_CALLBACK((*_eventBus), WindowCloseEvent, OnWindowCloseEvent);
		// Note: ApplicationCloseEvent is registered in Run() after all layers are initialized
	}

	static bool shouldClose = false;
	void Application::Run()
	{
		// Register ApplicationCloseEvent callback last, so all layer callbacks are called first
		REGISTER_CALLBACK((*_eventBus), RequestApplicationCloseEvent, OnRequestApplicationCloseEvent);
		REGISTER_CALLBACK((*_eventBus), ApplicationCloseEvent, OnApplicationCloseEvent);
		
		float deltaTime = 0.0f;
		float lastFrameTime = 0.0f;
		float currentFrameTime = 0.0f;

		while (!shouldClose)
		{
			_window->PollEvents();
			_eventBus->HandleEvents();

#ifdef USE_IMGUI
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();
#endif

			currentFrameTime = static_cast<float>(glfwGetTime());
			deltaTime = currentFrameTime - lastFrameTime;
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
			// if no renderer handles the glClear, we need to do it here
			//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			//glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

			_window->SwapBuffers();
		}
		Stop();
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


}