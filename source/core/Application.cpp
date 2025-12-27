#include "Application.h"
#include "glew/glew.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "event/ApplicationEvent.h"


namespace Core
{
	static void OnWindowResizeEvent(const WindowResizeEvent& e)
	{
		std::cout << "Window resized to: " << e.Width << "x" << e.Height << std::endl;
	}
	void Application::OnMouseDownEvent(const MouseDownEvent& e)
	{
		std::cout << "Mouse button down event: " << e.identifier << std::endl;
	}

	void Application::OnMouseUpEvent(const MouseUpEvent& e)
	{
		std::cout << "Mouse button up event: " << e.identifier << std::endl;
	}

	void Application::OnMouseMoveEvent(const MouseMoveEvent& e)
	{
		std::cout << "Mouse move event: " << e.posX << ", " << e.posY << std::endl;
	}

	void Application::OnMouseScrollEvent(const MouseScrollEvent& e)
	{
		std::cout << "Mouse scroll event: " << e.scrollX << ", " << e.scrollY << std::endl;
	}

	Application::Application(const ApplicationSpecs& specs) : _applicationSpecs(specs)
	{
		if (!glfwInit()) {
			std::cerr << "Failed to initialize GLFW" << std::endl;
			return;
		}
		_window = std::make_shared<Window>(_applicationSpecs.windowWidth, _applicationSpecs.windowHeight, _applicationSpecs.windowTitle, *_eventBus);

		if (glewInit() != GLEW_OK) {
			std::cerr << "Failed to initialize GLEW" << std::endl;
			return;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)_window->GetHandle(), true);
		ImGui_ImplOpenGL3_Init("#version 330");


		//register callbacks
		//manual callback example without the macro
		_eventBus->RegisterEventCallback<WindowResizeEvent>([](const Core::IEvent& e) {
			OnWindowResizeEvent((const WindowResizeEvent&)(e));
			});

		//or by using the MACRO
		REGISTER_CALLBACK((*_eventBus), MouseDownEvent, OnMouseDownEvent);
		REGISTER_CALLBACK((*_eventBus), MouseUpEvent, OnMouseUpEvent);
		REGISTER_CALLBACK((*_eventBus), MouseMoveEvent, OnMouseMoveEvent);
		REGISTER_CALLBACK((*_eventBus), MouseScrollEvent, OnMouseScrollEvent);

		_layerContext.Register<EventBus>(_eventBus);
	}

	void Application::Run()
	{
		float deltaTime = 0.0f;
		float lastFrameTime = 0.0f;
		float currentFrameTime = 0.0f;

		while (!_window->ShouldClose())
		{
			_window->PollEvents();
			_eventBus->HandleEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			currentFrameTime = static_cast<float>(glfwGetTime());
			deltaTime = currentFrameTime - lastFrameTime;
			lastFrameTime = currentFrameTime;

			for(const auto& layer : _applicationLayers)
			{
				layer->OnUpdate(deltaTime);
			}

			for (const auto& layer : _applicationLayers)
			{
				layer->OnRender();
			}

			ImGui::Render();
			ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			_window->SwapBuffers();
		}
		Stop();
	}

	void Application::Stop()
	{
		_window.reset();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwTerminate();
	}

	void Application::GetFrameBufferSize(int& width, int& height) const
	{
		_window->GetFramebufferSize(width, height);
	}
}