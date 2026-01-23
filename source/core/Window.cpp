#include "pch.h"
#include "Window.h"
#include <glew/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <iostream>
#include "event/EventBus.h"
#include "event/ApplicationEvent.h"
#include "event/MouseEvent.h"
#include "event/KeyEvent.h"
#include "Logger.h"


namespace Core
{
    void Window::SetCallBacks() const
    {
        //window events
        glfwSetWindowSizeCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, int width, int height)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
                self->Width = width;
                self->Height = height;
                self->_eventBus->PushEvent(WindowResizeEvent(width, height));
            });

        glfwSetWindowCloseCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
				self->_eventBus->PushEvent(WindowCloseEvent(window));
            });

        //Mouse events
        glfwSetMouseButtonCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, int button, int action, int mods)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
                switch (action)
                {
                case GLFW_PRESS:
                    self->_eventBus->PushEvent(MouseDownEvent(button));
                    break;
                case GLFW_RELEASE:
                    self->_eventBus->PushEvent(MouseUpEvent(button));
                    break;
                default: break;
                }
            });

        glfwSetCursorPosCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, double xPos, double yPos)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
                self->_eventBus->PushEvent(MouseMoveEvent(xPos, yPos));
            });

        glfwSetScrollCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
                self->_eventBus->PushEvent(MouseScrollEvent(xOffset, yOffset));
            });

        //Key events
        glfwSetKeyCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, int key, int code, int action, int mods)
            {
                auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

                switch (action)
                {
                case GLFW_PRESS:
                {
                    self->_eventBus->PushEvent(KeyDownEvent(key, false, mods));
                    break;
                }
                case GLFW_REPEAT:
                {
                    self->_eventBus->PushEvent(KeyDownEvent(key, true, mods));
                    break;
                }
                case GLFW_RELEASE:
                {
                    self->_eventBus->PushEvent(KeyUpEvent(key, mods));
                    break;
                }

                }
            });

        glfwSetCharCallback((GLFWwindow*)_glfwWindow, [](GLFWwindow* window, unsigned int codepoint)
            {
				auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
				self->_eventBus->PushEvent(KeyCharacterEvent(codepoint));
			});
	}

	Window::Window(int width, int height, const char* title, EventBus& eventBus)
		: Width(width), Height(height), _eventBus(&eventBus)
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

        _eventBus->PushEvent<WindowResizeEvent>(WindowResizeEvent(width, height));

        SetCallBacks();

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

    void* Window::GetNativeWindowHandle() const
    {
#ifdef _WIN32
        if (!_glfwWindow)
        {
            LOG_ERROR() << "GetNativeWindowHandle: _glfwWindow is null!";
            return nullptr;
        }
        
        void* hwnd = glfwGetWin32Window((GLFWwindow*)_glfwWindow);
        
        if (!hwnd)
        {
            LOG_ERROR() << "GetNativeWindowHandle: glfwGetWin32Window returned null!";
        }
        else
        {
            LOG_DEBUG() << "GetNativeWindowHandle: returning HWND " << hwnd;
        }
        
        return hwnd;
#else
        // For other platforms, return the GLFW window handle
        return _glfwWindow;
#endif
    }
}