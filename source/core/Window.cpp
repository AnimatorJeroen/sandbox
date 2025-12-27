#include "Window.h"
#include <glew/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "event/EventBus.h"
#include "event/ApplicationEvent.h"


namespace Core
{
	Window::Window(int width, int height, const char* title, EventBus& eventBus)
		: Width(width), Height(height), _eventBus(eventBus)
	{
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        _glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!_glfwWindow) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent((GLFWwindow*)_glfwWindow);
        glfwSwapInterval(1); // Enable vsync

        // Store this pointer for callback access
        glfwSetWindowUserPointer((GLFWwindow*)_glfwWindow, this);

		//Set window callbacks
        glfwSetWindowSizeCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, int width, int height)
        {
			auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
			self->Width = width;
			self->Height = height;
			self->_eventBus.PushEvent(WindowResizeEvent(width, height));
        });

        glfwSetMouseButtonCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, int button, int action, int mods)
        {
			auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            switch (action)
            {
            case GLFW_PRESS:
                self->_eventBus.PushEvent(MouseDownEvent(button));
                break;
            case GLFW_RELEASE:
                self->_eventBus.PushEvent(MouseUpEvent(button));
                break;
			default: break;
            }
        });

        glfwSetCursorPosCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, double xPos, double yPos)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
                self->_eventBus.PushEvent(MouseMoveEvent(xPos, yPos));
            });

        glfwSetScrollCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
                self->_eventBus.PushEvent(MouseScrollEvent(xOffset, yOffset));
            });
	}
    Window::~Window()
    {
		glfwDestroyWindow((GLFWwindow*)_glfwWindow);
    }
    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose((GLFWwindow*)_glfwWindow);
    }
    void Window::PollEvents() const
    {
        glfwPollEvents();
    }
    void Window::SwapBuffers() const
    {
        glfwSwapBuffers((GLFWwindow*)_glfwWindow);
	}

    void Window::GetFramebufferSize(int& width, int& height) const
    {
        glfwGetFramebufferSize((GLFWwindow*)_glfwWindow, &width, &height);
	}
}